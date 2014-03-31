#include "KeyhoteeMainWindow.hpp"

#include "ui_KeyhoteeMainWindow.h"

#include "connectionstatusframe.h"
#include "diagnosticdialog.h"
#include "GitSHA1.h"
#include "KeyhoteeApplication.hpp"
#include "MenuEditControl.hpp"
#include "public_key_address.hpp"

#include "AddressBook/AddressBookModel.hpp"
#include "AddressBook/authorization.hpp"
#include "AddressBook/ContactGui.hpp"
#include "AddressBook/ContactView.hpp"
#include "AddressBook/NewIdentityDialog.hpp"
#include "AddressBook/RequestAuthorization.hpp"

#include "BitShares/GitSHA2.h"
#include "BitShares/fc/GitSHA3.h"

#include "Mail/MailboxModel.hpp"
#include "Mail/maileditorwindow.hpp"

#include <fc/reflect/variant.hpp>
#include <fc/log/logger.hpp>

/// QT headers:
#include <QCompleter>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>

#ifdef Q_OS_MAC
//#include <qmacnativetoolbar.h>
#endif

extern bool        gMiningIsPossible;

KeyhoteeMainWindow* getKeyhoteeWindow()
{
  return TKeyhoteeApplication::getInstance()->getMainWindow();
}

enum SidebarItemRoles
{
  ContactIdRole = Qt::UserRole
};

enum TopLevelItemIndexes
{
  Mailboxes,
  Space2,
  Wallets,
  Space3,
  Contacts,
  Space4,
  Requests
};


KeyhoteeMainWindow::KeyhoteeMainWindow(const TKeyhoteeApplication& mainApp) :
  _identities_root(nullptr),
  _connectionProcessor(*this, bts::application::instance()->get_profile()),
  _currentMailbox(nullptr),
  _isClosing(false)
{
  ui = new Ui::KeyhoteeMainWindow;
  ui->setupUi(this);

  QString profileName = mainApp.getLoadedProfileName();

  QString title = QString("%1 v%2 (%3)").arg(mainApp.getAppName().c_str()).arg(mainApp.getVersionNumberString().c_str()).arg(profileName);
  setWindowTitle(title);
  setEnabledAttachmentSaveOption(false);
  setEnabledDeleteOption(false);
  setEnabledMailActions(false);
  setEnabledShareContactOption(false);

  QString settings_file = "keyhotee_";
  settings_file.append(profileName);
  setSettingsFile(settings_file);
  readSettings();

  connect(ui->contacts_page, &ContactsTable::contactOpened, this, &KeyhoteeMainWindow::openContactGui);
  connect(ui->contacts_page, &ContactsTable::contactDeleted, this, &KeyhoteeMainWindow::deleteContactGui);
  

#ifdef Q_OS_MAC
  //QMacNativeToolBar* native_toolbar = QtMacExtras::setNativeToolBar(ui->toolbar, true);
  ui->side_bar->setAttribute(Qt::WA_MacShowFocusRect, 0);
#endif /// Q_OS_MAC

  setupStatusBar();

  QWidget* empty = new QWidget();
  empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  ui->toolbar->addWidget(empty);

  _search_edit = new QLineEdit(ui->toolbar);
  ui->toolbar->addWidget(_search_edit);
  _search_edit->setMaximumSize(QSize(150, 22) );
  _search_edit->setAttribute(Qt::WA_MacShowFocusRect, 0);
  const char* search_style = "QLineEdit { " \
                             "padding-right: 20px; " \
                             "padding-left: 5px; " \
                             "background: url(:/images/search24x16.png);" \
                             "background-position: right;" \
                             "background-repeat: no-repeat;" \
                             "border: 1px solid gray;" \
                             "border-radius: 10px;}";
  _search_edit->setStyleSheet(search_style);
  _search_edit->setPlaceholderText(tr("Search") );

  QWidget* empty2 = new QWidget();
  empty->resize(QSize(10, 10) );
  ui->toolbar->addWidget(empty2);

  ui->actionEnable_Mining->setEnabled(gMiningIsPossible);
  ui->actionEnable_Mining->setVisible(gMiningIsPossible);

  ui->side_bar->setModificationsChecker (this);

  menuEdit = new MenuEditControl(this, ui->actionCopy, ui->actionCut, ui->actionPaste);
  //init ui->actionPaste
  onClipboardChanged();
  connect(QApplication::clipboard(), &QClipboard::changed, this, &KeyhoteeMainWindow::onClipboardChanged);

  // ---------------------- MenuBar
  // File
  connect(ui->actionExit, &QAction::triggered, this, &KeyhoteeMainWindow::onExit);
  // Edit
  connect(ui->actionCopy, &QAction::triggered, this, &KeyhoteeMainWindow::onCopy);
  connect(ui->actionCut, &QAction::triggered, this, &KeyhoteeMainWindow::onCut);
  connect(ui->actionPaste, &QAction::triggered, this, &KeyhoteeMainWindow::onPaste);
  connect(ui->actionSelect_All, &QAction::triggered, this, &KeyhoteeMainWindow::onSelectAll);
  // Identity
  connect(ui->actionNew_identity, &QAction::triggered, this, &KeyhoteeMainWindow::onNewIdentity);
  connect(ui->actionEnable_Mining, &QAction::toggled, this, &KeyhoteeMainWindow::onEnableMining);
  // Mail
  connect(ui->actionNew_Message, &QAction::triggered, this, &KeyhoteeMainWindow::newMailMessage);
  connect(ui->actionSave_attachement, &QAction::triggered, this, &KeyhoteeMainWindow::onSaveAttachement);
  // Contact
  connect(ui->actionNew_Contact, &QAction::triggered, this, &KeyhoteeMainWindow::addContact);
  connect(ui->actionSet_Icon, &QAction::triggered, this, &KeyhoteeMainWindow::onSetIcon);
  connect(ui->actionShow_Contacts, &QAction::triggered, this, &KeyhoteeMainWindow::showContacts);
  connect(ui->actionRequest_authorization, &QAction::triggered, this, &KeyhoteeMainWindow::onRequestAuthorization);
  connect(ui->actionShare_contact, &QAction::triggered, this, &KeyhoteeMainWindow::onShareContact);
  // Help
  connect(ui->actionDiagnostic, &QAction::triggered, this, &KeyhoteeMainWindow::onDiagnostic);
  connect(ui->actionAbout, &QAction::triggered, this, &KeyhoteeMainWindow::onAbout);

  connect(ui->splitter, &QSplitter::splitterMoved, this, &KeyhoteeMainWindow::sideBarSplitterMoved);
  connect(ui->side_bar, &TreeWidgetCustom::itemSelectionChanged, this, &KeyhoteeMainWindow::onSidebarSelectionChanged);
  connect(ui->side_bar, &TreeWidgetCustom::itemDoubleClicked, this, &KeyhoteeMainWindow::onSidebarDoubleClicked);
  connect(ui->side_bar, &TreeWidgetCustom::itemContactRemoved, this, &KeyhoteeMainWindow::onItemContactRemoved);

  connect(ui->side_bar, &TreeWidgetCustom::itemContextAcceptRequest, this, &KeyhoteeMainWindow::onItemContextAcceptRequest);
  connect(ui->side_bar, &TreeWidgetCustom::itemContextDenyRequest, this, &KeyhoteeMainWindow::onItemContextDenyRequest);
  connect(ui->side_bar, &TreeWidgetCustom::itemContextBlockRequest, this, &KeyhoteeMainWindow::onItemContextBlockRequest);
  
  //connect( _search_edit, SIGNAL(textChanged(QString)), this, SLOT(searchEditChanged(QString)) );
  connect(_search_edit, &QLineEdit::textChanged, this, &KeyhoteeMainWindow::searchEditChanged);

  auto space2 = ui->side_bar->topLevelItem(TopLevelItemIndexes::Space2);
  auto space3 = ui->side_bar->topLevelItem(TopLevelItemIndexes::Space3);
  auto space_flags = space2->flags() & (~Qt::ItemFlags(Qt::ItemIsSelectable) );
  space_flags |= Qt::ItemNeverHasChildren;
  space2->setFlags(space_flags);
  space3->setFlags(space_flags);

  //_identities_root = ui->side_bar->topLevelItem(TopLevelItemIndexes::Identities);
  _mailboxes_root = ui->side_bar->topLevelItem(TopLevelItemIndexes::Mailboxes);
  _contacts_root = ui->side_bar->topLevelItem(TopLevelItemIndexes::Contacts);
  _wallets_root = ui->side_bar->topLevelItem(TopLevelItemIndexes::Wallets);
  _requests_root = ui->side_bar->topLevelItem(TopLevelItemIndexes::Requests);

  _contacts_root->setExpanded(true);
  _requests_root->setExpanded(true);
  _requests_root->setHidden(true);
  //_identities_root->setExpanded(true);
  _mailboxes_root->setExpanded(true);
  _inbox_root = _mailboxes_root->child(Inbox);
  _drafts_root = _mailboxes_root->child(Drafts);
  _out_box_root = _mailboxes_root->child(Outbox);
  _sent_root = _mailboxes_root->child(Sent);

  _wallets_root->setExpanded(true);
  _bitcoin_root = _wallets_root->child(Bitcoin);
  _bitshares_root = _wallets_root->child(BitShares);
  _litecoin_root = _wallets_root->child(Litecoin);

  auto app = bts::application::instance();
  auto profile = app->get_profile();
  auto idents = profile->identities();

  auto addressbook = profile->get_addressbook();
  _addressbook_model = new AddressBookModel(this, addressbook);

  _inbox_model = new MailboxModel(this, profile, profile->get_inbox_db(), *_addressbook_model, false);
  _draft_model = new MailboxModel(this, profile, profile->get_draft_db(), *_addressbook_model, true);
  _pending_model = new MailboxModel(this, profile, profile->get_pending_db(), *_addressbook_model, false);
  _sent_model = new MailboxModel(this, profile, profile->get_sent_db(), *_addressbook_model, false);

  loadStoredRequests(profile->get_request_db());
  connect(_addressbook_model, &QAbstractItemModel::dataChanged, this,
    &KeyhoteeMainWindow::addressBookDataChanged);

  ui->contacts_page->setAddressBook(_addressbook_model);
  ui->new_contact->setAddressBook(_addressbook_model);

  ui->inbox_page->initial(_connectionProcessor, _inbox_model, Mailbox::Inbox, this);
  ui->draft_box_page->initial(_connectionProcessor, _draft_model, Mailbox::Drafts, this);
  ui->out_box_page->initial(_connectionProcessor, _pending_model, Mailbox::Outbox, this);
  ui->sent_box_page->initial(_connectionProcessor, _sent_model, Mailbox::Sent, this);  
  _mailboxesList.push_back (ui->inbox_page);
  _mailboxesList.push_back (ui->draft_box_page);
  _mailboxesList.push_back (ui->out_box_page);
  _mailboxesList.push_back (ui->sent_box_page);

  ui->widget_stack->setCurrentWidget(ui->inbox_page);
  ui->actionDelete->setShortcut(QKeySequence::Delete);
  connect(ui->actionDelete, SIGNAL(triggered()), ui->inbox_page, SLOT(onDeleteMail()));
  connect(ui->actionShow_details, SIGNAL(toggled(bool)), ui->inbox_page, SLOT(on_actionShow_details_toggled(bool)));
  connect(ui->actionReply, SIGNAL(triggered()), ui->inbox_page, SLOT(onReplyMail()));
  connect(ui->actionReply_all, SIGNAL(triggered()), ui->inbox_page, SLOT(onReplyAllMail()));
  connect(ui->actionForward, SIGNAL(triggered()), ui->inbox_page, SLOT(onForwardMail()));

  wlog("idents: ${idents}", ("idents", idents) );

  if(isIdentityPresent() == false )
  {
      ui->actionNew_Message->setEnabled(false);
      ui->actionRequest_authorization->setEnabled(false);
  }

  for (size_t i = 0; i < idents.size(); ++i)
  {
     try {
          app->mine_name(idents[i].dac_id_string,
                         profile->get_keychain().get_identity_key(idents[i].dac_id_string).get_public_key(),
                         idents[i].mining_effort);
     } 
     catch ( const fc::exception& e )
     {
        wlog( "${e}", ("e",e.to_detail_string()) );
     }
  }
  app->set_mining_intensity(0);
  ui->actionEnable_Mining->setChecked(app->get_mining_intensity() != 0);
  _addressbook = profile->get_addressbook();

  /*
     auto abook  = profile->get_addressbook();
     auto contacts = abook->get_known_bitnames();
     for( auto itr = contacts.begin(); itr != contacts.end(); ++itr )
   {
      auto new_contact_item = new QTreeWidgetItem(_contacts_root, (QTreeWidgetItem::ItemType)ContactItem );

      auto id_rec = app->lookup_name( *itr );
      if( !id_rec )
    {
         new_contact_item->setText( 0, (*itr + " [unknown]").c_str() );
    }
      else
    {
         new_contact_item->setText( 0, (*itr + " [" + std::to_string(id_rec->repute)+"]" ).c_str() );
    }
   }
   */

  QAction* actionMenu = new QAction(tr("Keyhotee"), this);
  actionMenu->setCheckable(true);
  this->setMenuWindow(ui->menuWindow);
  this->registration(actionMenu);
  actionMenu->setVisible(false);
}

KeyhoteeMainWindow::~KeyhoteeMainWindow()
  {
  delete menuEdit;
  delete ui;
  }

void KeyhoteeMainWindow::activateMailboxPage(Mailbox* mailBox)
  {
  ui->widget_stack->setCurrentWidget(mailBox);
  connect(ui->actionDelete, SIGNAL(triggered()), mailBox, SLOT(onDeleteMail()));
  connect(ui->actionShow_details, SIGNAL(toggled(bool)), mailBox, SLOT(on_actionShow_details_toggled(bool)));
  connect(ui->actionReply, SIGNAL(triggered()), mailBox, SLOT(onReplyMail()));
  connect(ui->actionReply_all, SIGNAL(triggered()), mailBox, SLOT(onReplyAllMail()));
  connect(ui->actionForward, SIGNAL(triggered()), mailBox, SLOT(onForwardMail()));
  bool checked = mailBox->isShowDetailsHidden() == false;
  ui->actionShow_details->setChecked(checked);

  _currentMailbox = mailBox;
  if(nullptr != _currentMailbox)
    _currentMailbox->checksendmailbuttons();
  setEnabledAttachmentSaveOption(_currentMailbox->isAttachmentSelected());
  setEnabledDeleteOption (_currentMailbox->isSelection());
  setEnabledMailActions(_currentMailbox->isOneEmailSelected());
  }

void KeyhoteeMainWindow::addContact()
{
  if (checkSaving())
  {
      connect(ui->new_contact, &ContactView::savedNewContact, this, &KeyhoteeMainWindow::onSavedNewContact);
      connect(ui->new_contact, &ContactView::savedNewContact, ui->contacts_page, &ContactsTable::onSavedNewContact);
      connect(ui->new_contact, &ContactView::canceledNewContact, this, &KeyhoteeMainWindow::onCanceledNewContact);
      connect(ui->new_contact, &ContactView::canceledNewContact, ui->contacts_page, &ContactsTable::onCanceledNewContact);

      enableMenu(false);
      ui->new_contact->setAddingNewContact(true);
      ui->new_contact->setContact(Contact());    
      ui->contacts_page->addNewContact(*ui->new_contact);
      ui->widget_stack->setCurrentWidget(ui->contacts_page);
      //ui->widget_stack->setCurrentWidget( ui->new_contact);
  }
}

void KeyhoteeMainWindow::addToContacts(const bts::addressbook::wallet_contact& wallet_contact)
{
  addContact();
  std::string public_key_string = public_key_address(wallet_contact.public_key);
  ui->new_contact->setPublicKey(public_key_string.c_str());
}

void KeyhoteeMainWindow::addToContacts(bool silent, std::list<Contact> &contacts)
{
  if (silent)
  {
    for (const auto& contact : contacts)
    {
      _addressbook_model->storeContact(contact);
      showContacts();
    }
  }
  else
  {
    //only 1 contact on !silent mode
    assert(contacts.size() == 1);
    addContact();
    const Contact &contact = contacts.front();
    ui->new_contact->setContactFromvCard(contact);
  }
}

void KeyhoteeMainWindow::sideBarSplitterMoved(int pos, int index)
{
  if (pos <= 5)
    ui->splitter->setHandleWidth(5);
  else
    ui->splitter->setHandleWidth(0);
}

void KeyhoteeMainWindow::addressBookDataChanged(const QModelIndex& top_left, const QModelIndex& bottom_right,
                                                const QVector<int>& roles)
{
  const Contact& changed_contact = _addressbook_model->getContact(top_left);
  auto           itr = _contact_guis.find(changed_contact.wallet_index);
  if (itr != _contact_guis.end() )
    itr->second.updateTreeItemDisplay();
}

void KeyhoteeMainWindow::searchEditChanged(QString search_string)
{
  auto     current_widget = ui->widget_stack->currentWidget();
  Mailbox* mailbox = dynamic_cast<Mailbox*>(current_widget);
  if (mailbox)
  {
    mailbox->searchEditChanged(search_string);
    return;
  }
  ContactsTable* contacts_table = dynamic_cast<ContactsTable*>(current_widget);
  if (contacts_table)
  {
    contacts_table->searchEditChanged(search_string);
    return;
  }
}

bool KeyhoteeMainWindow::isSelectedContactGui(ContactGui* contactGui)
{
  QList<QTreeWidgetItem*> selected_items = ui->side_bar->selectedItems();
  if (selected_items.size() == 1)
    return selected_items[0] == contactGui->_tree_item;
  return false;
}

void KeyhoteeMainWindow::onSidebarSelectionChanged()
{
  QList<QTreeWidgetItem*> selected_items = ui->side_bar->selectedItems();
  if (selected_items.size() )
  {
    disconnect(ui->actionDelete, SIGNAL(triggered()), this, SLOT(onRemoveContact()));
    disconnect(ui->actionDelete, SIGNAL(triggered()), this, SLOT(onDeleteAuthorizationItem()));
    disconnect(ui->actionShow_details, SIGNAL(toggled(bool)), ui->contacts_page, SLOT(on_actionShow_details_toggled(bool)));

    for (Mailbox* mailBox : _mailboxesList)
    {
      disconnect(ui->actionDelete, SIGNAL(triggered()), mailBox, SLOT(onDeleteMail()));
      disconnect(ui->actionReply, SIGNAL(triggered()), mailBox, SLOT(onReplyMail()));
      disconnect(ui->actionReply_all, SIGNAL(triggered()), mailBox, SLOT(onReplyAllMail()));
      disconnect(ui->actionForward, SIGNAL(triggered()), mailBox, SLOT(onForwardMail()));
      disconnect(ui->actionShow_details, SIGNAL(toggled(bool)), mailBox, SLOT(on_actionShow_details_toggled(bool)));
    }

    setEnabledDeleteOption (false);
    setEnabledAttachmentSaveOption(false);
    setEnabledMailActions(false);
    setEnabledShareContactOption(false);
    _currentMailbox = nullptr;
    ui->actionShow_details->setEnabled(true);

    QTreeWidgetItem* selectedItem = selected_items.first();

    if (selectedItem->type() == ContactItem)
    {
      auto con_id = selected_items[0]->data(0, ContactIdRole).toInt();
      openContactGui(con_id);
      ui->contacts_page->selectRow(con_id);
      connect(ui->actionDelete, SIGNAL(triggered()), this, SLOT(onRemoveContact()));
      connect(ui->actionShow_details, SIGNAL(toggled(bool)), ui->contacts_page, SLOT(on_actionShow_details_toggled(bool)));
      if(ui->contacts_page->isShowDetailsHidden())
        ui->actionShow_details->setChecked(false);
      else
        ui->actionShow_details->setChecked(true);

      refreshMenuOptions();
    }
    else if (selectedItem->type() == IdentityItem)
    {
      selectIdentityItem(selectedItem);
    }
    else if (selectedItem == _contacts_root)
    {
      showContacts();
      connect(ui->actionDelete, SIGNAL(triggered()), this, SLOT(onRemoveContact()));
      connect(ui->actionShow_details, SIGNAL(toggled(bool)), ui->contacts_page, SLOT(on_actionShow_details_toggled(bool)));
      if (ui->contacts_page->isShowDetailsHidden())
        ui->actionShow_details->setChecked(false);
      else
        ui->actionShow_details->setChecked(true);      

      refreshMenuOptions();
    }
    else if(selectedItem == _requests_root)
    {
    }
    if (selectedItem->type() == RequestItem)
    {
      showAuthorizationItem(static_cast<AuthorizationItem*>(selectedItem));
      connect(ui->actionDelete, SIGNAL(triggered()), this, SLOT(onDeleteAuthorizationItem()));
      setEnabledDeleteOption(true);
      ui->actionShow_details->setEnabled(false);
    }
    //else if( selected_items[0] == _identities_root )
    //{
    //}
    /// For mailboxes root just select inbox root
    else if (selectedItem == _mailboxes_root || selectedItem == _inbox_root)
      {
      activateMailboxPage(ui->inbox_page);
      }
    else if (selectedItem == _drafts_root)
      {
      activateMailboxPage(ui->draft_box_page);
      }
    else if (selectedItem == _out_box_root)
      {
      activateMailboxPage(ui->out_box_page);
      }
    else if (selectedItem == _sent_root)
      {
      activateMailboxPage(ui->sent_box_page);
      }
    else if (selected_items[0] == _wallets_root)
    {
      ui->widget_stack->setCurrentWidget(ui->wallets);
    }
    else if (selected_items[0] == _bitcoin_root)
    {
      ui->widget_stack->setCurrentWidget(ui->wallets);
    }
    else if (selected_items[0] == _bitshares_root)
    {
      ui->widget_stack->setCurrentWidget(ui->wallets);
    }
    else if (selected_items[0] == _litecoin_root)
    {
      ui->widget_stack->setCurrentWidget(ui->wallets);
    }
  }
}

void KeyhoteeMainWindow::onSidebarDoubleClicked()
{
  QList<QTreeWidgetItem*> selected_items = ui->side_bar->selectedItems();
  if (selected_items.size() )
  {
    if (selected_items[0]->type() == ContactItem)
    {
      ui->contacts_page->selectChat ();
    }
  }
}
void KeyhoteeMainWindow::selectContactItem(QTreeWidgetItem* item)
{}

void KeyhoteeMainWindow::selectIdentityItem(QTreeWidgetItem* item)
{}

// Menu File
void KeyhoteeMainWindow::onExit()
{
  qApp->closeAllWindows();
}

// Menu Edit
void KeyhoteeMainWindow::onCopy()
{
  QWidget *focused = focusWidget ();
  if (focused == nullptr)
    return;

  if(focused == ui->side_bar) //TreeView focused
  {
    if (ui->widget_stack->currentWidget () == ui->contacts_page)
      ui->contacts_page->copy ();
  }
  //contact list
  else if(focused == ui->contacts_page->getContactsTableWidget())
  {
    ui->contacts_page->copy ();
  }
  else
  {
    menuEdit->copy();
  }
}

void KeyhoteeMainWindow::onCut()
{
  menuEdit->cut();
}

void KeyhoteeMainWindow::onPaste()
{
  menuEdit->paste();
}

void KeyhoteeMainWindow::onSelectAll()
{
  QWidget *focused = focusWidget ();

  if (focused == nullptr)
    return;

  if(ui->side_bar == focused) //TreeView focused
  {
    if (ui->widget_stack->currentWidget () == ui->contacts_page)
      ui->contacts_page->selectAll ();
    else if (ui->widget_stack->currentWidget () == _currentMailbox)
      _currentMailbox->selectAll ();
    else if (ui->widget_stack->currentWidget () == ui->wallets)
      ; //ui->wallets->selectAll ();
    else
      assert (0);
  }
  else
  {
    menuEdit->selectAll();
  }
}

// Menu Identity
void KeyhoteeMainWindow::onNewIdentity()
{
   NewIdentityDialog* ident_dialog = new NewIdentityDialog(this);

   QObject::connect(ident_dialog, SIGNAL(identityadded()),
                    this, SLOT(enableNewMessageIcon()));

   ident_dialog->show();
}

bool KeyhoteeMainWindow::isIdentityPresent()
{
    auto app = bts::application::instance();
    auto profile = app->get_profile();

    auto idents = profile->identities();
    return (idents.size() != 0);
}

void KeyhoteeMainWindow::enableNewMessageIcon()
{
    if(isIdentityPresent() == true ) {
         ui->actionNew_Message->setEnabled(true);
         ui->actionRequest_authorization->setEnabled(true);
         emit checkSendMailSignal();
         if(nullptr != _currentMailbox)
           _currentMailbox->checksendmailbuttons();
    }
}

void KeyhoteeMainWindow::onEnableMining(bool enabled)
{
  auto app = bts::application::instance();
  app->set_mining_intensity(enabled ? 100 : 0);
}

// Menu Mail
void KeyhoteeMainWindow::onSaveAttachement()
{
  assert (_currentMailbox != nullptr);
  _currentMailbox->saveAttachment();
}

// Menu Contact
void KeyhoteeMainWindow::onSetIcon()
{
  notSupported();
}

void KeyhoteeMainWindow::onRequestAuthorization()
{
  RequestAuthorization *request = new RequestAuthorization(this);
  request->setAddressBook(_addressbook_model);
  request->show();
}

void KeyhoteeMainWindow::displayDiagnosticLog()
{
  DiagnosticDialog diagnoslic_dialog;
  diagnoslic_dialog.setModal(true);
  diagnoslic_dialog.exec();
}
// Menu Help
void KeyhoteeMainWindow::onDiagnostic()
{
  displayDiagnosticLog();
}

void KeyhoteeMainWindow::onAbout()
{
  QString title(tr("About "));
  title += TKeyhoteeApplication::getInstance()->getAppName().c_str();
  QString text;
  text = tr("<p align='center'><b>");
  text += TKeyhoteeApplication::getInstance()->getAppName().c_str();
/// Commented out to avoid difference against install package version.
  text += tr(" version ");
  text += tr(TKeyhoteeApplication::getInstance()->getVersionNumberString().c_str());
  text += tr("</b><br/><br/>");
  /// Build tag: <a href="https://github.com/InvictusInnovations/keyhotee/commit/xxxx">xxxx</a>
  text += tr("<strong>keyhotee</strong> built from revision: <a href=\"https://github.com/InvictusInnovations/keyhotee/commit/");
  text += tr(g_GIT_SHA1);
  text += tr("\">");
  text += tr(std::string(g_GIT_SHA1).substr(0, 10).c_str());
  text += tr("</a>");
  text += tr(" (<em>");
  text += tr(fc::get_approximate_relative_time_string(fc::time_point_sec(g_GIT_UNIX_TIMESTAMP1)).c_str());
  text += tr("</em>)");
  text += tr("<br/>");
  text += tr("<br/>");
  text += tr("<strong>BitShares</strong> built from revision: <a href=\"https://github.com/InvictusInnovations/BitShares/commit/");
  text += tr(g_GIT_SHA2);
  text += tr("\">");
  text += tr(std::string(g_GIT_SHA2).substr(0, 10).c_str());
  text += tr("</a>");
  text += tr(" (<em>");
  text += tr(fc::get_approximate_relative_time_string(fc::time_point_sec(g_GIT_UNIX_TIMESTAMP2)).c_str());
  text += tr("</em>)");
  text += tr("<br/>");
  text += tr("<br/>");
  text += tr("<strong>fc</strong> built from revision: <a href=\"https://github.com/InvictusInnovations/fc/commit/");
  text += tr(g_GIT_SHA3);
  text += tr("\">");
  text += tr(std::string(g_GIT_SHA3).substr(0, 10).c_str());
  text += tr("</a>");
  text += tr(" (<em>");
  text += tr(fc::get_approximate_relative_time_string(fc::time_point_sec(g_GIT_UNIX_TIMESTAMP3)).c_str());
  text += tr("</em>)");
  text += tr("<br/>");
  text += tr("<br/>");
  text += tr("Invictus Innovations Inc<br/>");
  text += tr("<a href=\"http://invictus-innovations.com/keyhotee/\">http://invictus-innovations.com/keyhotee/</a>");
  text += tr("<br/></p>");

  QMessageBox::about(this, title, text);
}

void KeyhoteeMainWindow::showContacts() const
{
  ui->side_bar->setCurrentItem(_contacts_root);
  ui->widget_stack->setCurrentWidget(ui->contacts_page);
}

void KeyhoteeMainWindow::newMailMessage()
{
  MailEditorMainWindow* mailWindow = new MailEditorMainWindow(this, *_addressbook_model,
    _connectionProcessor, true);

  mailWindow->show();
}

void KeyhoteeMainWindow::newMailMessageTo(const Contact& contact)
{
  MailEditorMainWindow* mailWindow = new MailEditorMainWindow(this, *_addressbook_model,
    _connectionProcessor, true);

  IMailProcessor::TRecipientPublicKeys toList, emptyList;
  toList.push_back(contact.public_key);
  mailWindow->SetRecipientList(toList, emptyList, emptyList);
  mailWindow->show();
}

void KeyhoteeMainWindow::shareContact(QList<const Contact*>& contacts)
{
  assert (contacts.size());

  MailEditorMainWindow* mailWindow = new MailEditorMainWindow(this, *_addressbook_model,
    _connectionProcessor, true);

  for(const Contact* contact : contacts)
  {
    mailWindow->addContactCard (*contact);
  }

  mailWindow->show();
}

ContactGui* KeyhoteeMainWindow::getContactGui(int contact_id)
{
  auto itr = _contact_guis.find(contact_id);
  if (itr != _contact_guis.end() )
    return &(itr->second);
  return nullptr;
}

void KeyhoteeMainWindow::openContactGui(int contact_id)
{
  ui->actionShow_details->setEnabled (true);
  if (contact_id == -1)    // TODO: define -1 as AddressBookID
  {
    showContacts();
    return;
  }
  else
  {
    auto contact_gui = createContactGuiIfNecessary(contact_id);
    showContactGui(*contact_gui);
    contact_gui->updateTreeItemDisplay();
  }
}

ContactGui* KeyhoteeMainWindow::createContactGuiIfNecessary(int contact_id)
{
  ContactGui* contact_gui = getContactGui(contact_id);
  if (!contact_gui)
  {
    createContactGui(contact_id);
    contact_gui = getContactGui(contact_id);
  }
  //DLNFIX not too sure we're doing everything in this call that's necessary
  // (or that this is the proper call to do it). Anywyas, this is quick fix
  // by yuvaraj that should be replaced eventually once we get a proper
  // signal emitted when registration occurs for a displayed KeyhoteeId.
  contact_gui->_view->checkKeyhoteeIdStatus();

  contact_gui->_view->checkAuthorizationStatus();
  contact_gui->_view->checkSendMailButton();
  if(nullptr != _currentMailbox)
    _currentMailbox->checksendmailbuttons();
  return contact_gui;
}

void KeyhoteeMainWindow::createContactGui(int contact_id)
{
  //DLNFIX2 maybe cleanup/refactor ContactGui construction later
  auto new_contact_item = new QTreeWidgetItem(_contacts_root,
                                              (QTreeWidgetItem::ItemType)ContactItem);
  new_contact_item->setData(0, ContactIdRole, contact_id);

  auto       view = new ContactView(ui->widget_stack);

  QObject::connect(this, SIGNAL(checkSendMailSignal()),
                   view, SLOT(checkSendMailButton()));
  //add new contactGui to map
  _contact_guis[contact_id] = ContactGui(new_contact_item, view);

  view->setAddressBook(_addressbook_model);
  const Contact& contact = _addressbook_model->getContactById(contact_id);
  view->setContact(contact);
  ui->contacts_page->addContactView(*view);
}

void KeyhoteeMainWindow::showContactGui(ContactGui& contact_gui)
{
  if (checkSaving())
  {
    ui->side_bar->setCurrentItem(contact_gui._tree_item);
    //ui->widget_stack->setCurrentWidget( contact_gui._view );
    ui->widget_stack->setCurrentWidget(ui->contacts_page);
    ui->contacts_page->showView(*contact_gui._view);
    if (contact_gui.isChatVisible() || contact_gui._unread_msg_count)
      contact_gui._view->onChat();
  }
}

void KeyhoteeMainWindow::deleteContactGui(int contact_id)
{
  ContactGui* contact_gui = getContactGui(contact_id);
  if(contact_gui != nullptr)
    {
    _contacts_root->removeChild(contact_gui->_tree_item);
    _contact_guis.erase(contact_id);
    if(nullptr != _currentMailbox)
      _currentMailbox->checksendmailbuttons();
    }

  assert(_contact_guis.find(contact_id) == _contact_guis.end());
}

void KeyhoteeMainWindow::createAuthorizationItem(const TAuthorizationMessage& msg,
                                                 const TStoredMailMessage& header)
{
  AuthorizationView *view = new AuthorizationView(_connectionProcessor, msg, header);
  view->setAddressBook(_addressbook_model);

  bool add_to_root = false;
  QTreeWidgetItem *item = nullptr;
  item = findExistSenderItem(header.from_key, add_to_root);

  if(add_to_root)
  {
    AuthorizationView *view_root = new AuthorizationView(_connectionProcessor, msg, header);

    AuthorizationItem *authorization_root_item = new AuthorizationItem(view_root,
      item, (QTreeWidgetItem::ItemType)RequestItem);

    authorization_root_item->setIcon(0, QIcon(":/images/request_authorization.png") );
    authorization_root_item->setFromKey(header.from_key);

    QString full_name = QString::fromStdString(msg.from_first_name);
    full_name += " " + QString::fromStdString(msg.from_last_name);
    authorization_root_item->setText(0, full_name);
    
    authorization_root_item->setHidden(false);
    authorization_root_item->setData(0, Qt::UserRole, false);   // 1 child, information for contextmenu
    view_root->setOwnerItem(authorization_root_item);

    connect(view_root, &AuthorizationView::itemAcceptRequest, this, &KeyhoteeMainWindow::onItemAcceptRequest);
    connect(view_root, &AuthorizationView::itemDenyRequest, this, &KeyhoteeMainWindow::onItemDenyRequest);
    connect(view_root, &AuthorizationView::itemBlockRequest, this, &KeyhoteeMainWindow::onItemBlockRequest);

    item = authorization_root_item;
  }

  AuthorizationItem *authorization_item = new AuthorizationItem(view,
    item, (QTreeWidgetItem::ItemType)RequestItem);

  authorization_item->setIcon(0, QIcon(":/images/request_authorization.png") );
  authorization_item->setFromKey(header.from_key);
  QDateTime dateTime;
  dateTime.setTime_t(header.from_sig_time.sec_since_epoch());
  authorization_item->setText(0, dateTime.toString(Qt::SystemLocaleShortDate));
  authorization_item->setHidden(false);
  view->setOwnerItem(authorization_item);

  connect(view, &AuthorizationView::itemAcceptRequest, this, &KeyhoteeMainWindow::onItemAcceptRequest);
  connect(view, &AuthorizationView::itemDenyRequest, this, &KeyhoteeMainWindow::onItemDenyRequest);
  connect(view, &AuthorizationView::itemBlockRequest, this, &KeyhoteeMainWindow::onItemBlockRequest);


  ui->widget_stack->addWidget(view);

  _requests_root->setHidden(false);
  if(!add_to_root)
  {
    item->setExpanded(true);
    item->setData(0, Qt::UserRole, true);   // multi children, information for contextmenu
  }
}

QTreeWidgetItem* 
KeyhoteeMainWindow::findExistSenderItem(AuthorizationItem::TPublicKey from_key, bool &to_root)
{
  if(_requests_root->childCount() > 0)
  {
    for(int i=0; i<_requests_root->childCount(); i++)
    {
      QTreeWidgetItem* child_item = _requests_root->child(i);
      if(static_cast<AuthorizationItem*>(child_item)->isEqual(from_key))
        return child_item;
    }
  }
  
  to_root = true;
  return _requests_root;
}

void KeyhoteeMainWindow::showAuthorizationItem(AuthorizationItem *item)
{
    ui->widget_stack->setCurrentWidget(item->getView());
}

void KeyhoteeMainWindow::deleteAuthorizationItem(AuthorizationItem *item)
{
  ui->widget_stack->removeWidget(item->getView());
  QTreeWidgetItem* item_parent = item->parent();
  item_parent->removeChild(item);

  if(item_parent != _requests_root)
  {
    if(item_parent->childCount() == 0)
      item_parent->parent()->removeChild(item_parent);
    else if(item_parent->childCount() == 1)
      item_parent->setData(0, Qt::UserRole, false);   // 1 child, information for contextmenu
  }

  if(_requests_root->childCount() == 0)
    _requests_root->setHidden(true);
}

void KeyhoteeMainWindow::processResponse(const TAuthorizationMessage& msg,
                                         const TStoredMailMessage& header)
{
  Authorization *authorization = new Authorization(_connectionProcessor, msg, header);
  authorization->processResponse();
}
void KeyhoteeMainWindow::loadStoredRequests(bts::bitchat::message_db_ptr request_db)
{
  try
  {
    auto headers = request_db->fetch_headers(bts::bitchat::private_contact_request_message::type);
    for(uint32_t i = 0; i < headers.size(); ++i)
    {
      auto header = headers[i];
      auto raw_data = request_db->fetch_data(header.digest);
      auto msg = fc::raw::unpack<bts::bitchat::private_contact_request_message>(raw_data);
      OnReceivedAuthorizationMessage(msg, header);
    }
  }
  catch(const fc::exception& e)
  {
    elog("${e}", ("e", e.to_detail_string()));
  }
}

void KeyhoteeMainWindow::onItemAcceptRequest(AuthorizationItem *item)
{
  deleteAuthorizationItem(item);
}

void KeyhoteeMainWindow::onItemDenyRequest(AuthorizationItem *item)
{
  deleteAuthorizationItem(item);
}

void KeyhoteeMainWindow::onItemBlockRequest(AuthorizationItem *item)
{
  deleteAuthorizationItem(item);
}

void KeyhoteeMainWindow::onItemContextAcceptRequest(QTreeWidgetItem *item)
{
  if(item->childCount() > 0)
  {
    while(item->child(0))
      static_cast<AuthorizationItem*>(item)->getView()->onAccept();
  }
  else
    static_cast<AuthorizationItem*>(item)->getView()->onAccept();

//  deleteAuthorizationItem(static_cast<AuthorizationItem*>(item));
}

void KeyhoteeMainWindow::onItemContextDenyRequest(QTreeWidgetItem *item)
{
  if(QMessageBox::question(this, tr("Deny request"), tr("Are you sure you want to deny selected request(s) for authorization?")) == QMessageBox::Button::No)
    return;

  if(item->childCount() > 0)
  {
    while(item->child(0))
      static_cast<AuthorizationItem*>(item)->getView()->onDeny();
  }
  else
  {
    static_cast<AuthorizationItem*>(item)->getView()->onDeny();
  }
}

void KeyhoteeMainWindow::onItemContextBlockRequest(QTreeWidgetItem *item)
{
  if(QMessageBox::question(this, tr("Block request"),
     tr("Are you sure you want to block selected request(s) for authorization?")) == QMessageBox::Button::No)
    return;

  if(item->childCount() > 0)
  {
    while(item->child(0))
      static_cast<AuthorizationItem*>(item)->getView()->onBlock();
  }
  else
  {
    static_cast<AuthorizationItem*>(item)->getView()->onBlock();
  }
}

void KeyhoteeMainWindow::onDeleteAuthorizationItem()
{
  QList<QTreeWidgetItem*> selected_items = ui->side_bar->selectedItems();
  if (selected_items.size() && ui->side_bar->hasFocus())
  {
    QTreeWidgetItem* selectedItem = selected_items.first();
    if (selectedItem->type() == RequestItem)
      onItemContextDenyRequest(selectedItem);
  }
}

void KeyhoteeMainWindow::setupStatusBar()
{
  QStatusBar*             sb = statusBar();
  TConnectionStatusFrame* cs = new TConnectionStatusFrame(_connectionProcessor);
  sb->addPermanentWidget(cs);
}

void KeyhoteeMainWindow::OnReceivedChatMessage(const TContact& sender, const TChatMessage& msg,
  const TTime& timeSent)
  {
  QDateTime dateTime;
  dateTime.setTime_t(timeSent.sec_since_epoch());
  QString fromLabel = QString::fromStdString(sender.get_display_name());
  QString msgBody = QString::fromStdString(msg.msg);
  auto contact_gui = createContactGuiIfNecessary(sender.wallet_index);
  contact_gui->receiveChatMessage(fromLabel, msgBody, dateTime);
  }

void KeyhoteeMainWindow::OnReceivedAuthorizationMessage(const TAuthorizationMessage& msg,
                                                        const TStoredMailMessage& header)
  {
  if(msg.status == bts::bitchat::authorization_status::request)
    createAuthorizationItem(msg, header);
  else
    processResponse(msg, header);
  }

void KeyhoteeMainWindow::OnReceivedMailMessage(const TStoredMailMessage& msg)
  {
  _inbox_model->addMailHeader(msg);
  }

void KeyhoteeMainWindow::OnReceivedUnsupportedMessage(const TDecryptedMessage& msg)
{
  QMessageBox::warning(this, tr("Keyhotee Warning"),
    tr("Received unsupported message.\nIt's strongly recommended to update application"));
}

void KeyhoteeMainWindow::OnMessageSaving()
{
  statusBar()->showMessage(tr("Saving a mail message into Draft folder..."), 1000);
}

void KeyhoteeMainWindow::OnMessageSaved(const TStoredMailMessage& msg,
                                        const TStoredMailMessage* overwrittenOne)
{
  if(overwrittenOne != nullptr)
    _draft_model->replaceMessage(*overwrittenOne, msg);
  else
    _draft_model->addMailHeader(msg);

  ui->draft_box_page->refreshMessageViewer();

  statusBar()->showMessage(tr("Mail message has been saved into Draft folder."), 3000);
}

void KeyhoteeMainWindow::OnMessageGroupPending(unsigned int count)
{
  /// Probably nothing to do here until we would like also display some notification
}

void KeyhoteeMainWindow::OnMessagePending(const TStoredMailMessage& msg,
  const TStoredMailMessage* savedDraftMsg)
{
  if(savedDraftMsg != nullptr)
    ui->draft_box_page->removeMessage(*savedDraftMsg);
  else
    _pending_model->addMailHeader(msg);

  ui->draft_box_page->refreshMessageViewer();
  ui->out_box_page->refreshMessageViewer();
}

void KeyhoteeMainWindow::OnMessageGroupPendingEnd()
{
  /// Probably nothing to do here until we would like also display some notification
}

void KeyhoteeMainWindow::OnMessageSendingStart()
{
  statusBar()->showMessage(tr("Starting mail transmission..."), 1000);
}

void KeyhoteeMainWindow::OnMessageSent(const TStoredMailMessage& pendingMsg,
  const TStoredMailMessage& sentMsg)
{
  ui->out_box_page->removeMessage(pendingMsg);
  ui->out_box_page->refreshMessageViewer();
  _sent_model->addMailHeader(sentMsg);
  ui->sent_box_page->refreshMessageViewer();
}

void KeyhoteeMainWindow::OnMessageSendingEnd()
{
  statusBar()->showMessage(tr("All mail messages sent."), 3000);
}

void KeyhoteeMainWindow::OnMissingSenderIdentity(const TRecipientPublicKey& senderId,
  const TPhysicalMailMessage& msg)
{
  public_key_address pkAddress(senderId);
  std::string pkAddressText(pkAddress);

  QMessageBox::warning(this, tr("Mail send"),
    tr("Following sender identity specified in a pending mail message doesn't exist anymore:\n") +
    QString(pkAddressText.c_str()));
}

void KeyhoteeMainWindow::notSupported()
{
  QMessageBox::warning(this, "Warning", "Not supported");
}

void KeyhoteeMainWindow::onCanceledNewContact()
{
  enableMenu(true);
  onSidebarSelectionChanged();
}

void KeyhoteeMainWindow::onSavedNewContact(int idxNewContact)
{
  enableMenu(true);
}

void KeyhoteeMainWindow::enableMenu(bool enable)
{
  ui->actionShow_details->setEnabled (enable);
  setEnabledDeleteOption (enable);
  setEnabledMailActions(enable);
  ui->actionNew_Contact->setEnabled (enable);
  ui->actionShow_Contacts->setEnabled (enable);
  _search_edit->setEnabled (enable);  
  setEnabledShareContactOption(enable);
}

void KeyhoteeMainWindow::closeEvent(QCloseEvent *closeEvent)
{
  if (checkSaving() && stopMailTransmission())
    {
    _isClosing = true;

    writeSettings ();
    for (Mailbox* mailBox : _mailboxesList)
    {
      mailBox->writeSettings ();
    }

    closeEvent->accept();
    ATopLevelWindowsContainer::closeEvent(closeEvent);
    }
  else
    {
    closeEvent->ignore();
    }
}

bool KeyhoteeMainWindow::stopMailTransmission()
  {
  bool canBreak = false;

  bool transferringMail = !_connectionProcessor.CanQuit(&canBreak);
  if(!transferringMail)
    return true;

  if(canBreak)
    {
    QMessageBox::StandardButton ret = QMessageBox::question(this, tr("Application"),
      tr("Sending email messages is in progress.\nDo you want to stop it and quit the application ?"),
        QMessageBox::Yes | QMessageBox::No);

    if(ret == QMessageBox::Yes)
      {
      _connectionProcessor.CancelTransmission();
      return true;
      }
    }
  else
    {
    QMessageBox::about(this, tr("Application"),
      tr("Receiving email message(s) is in progress.\nPlease wait until transmission finishes."));
    return false;
    }

  return false;
  }

bool KeyhoteeMainWindow::checkSaving() const
{
  if (ui->widget_stack->currentWidget () == ui->contacts_page)
    return ui->contacts_page->checkSaving();
  return true;
}

bool KeyhoteeMainWindow::canContinue() const
  {
  return checkSaving();
  }

void KeyhoteeMainWindow::onItemContactRemoved (QTreeWidgetItem& itemContact)
  {
  itemContact.setHidden (true);
  ui->contacts_page->contactRemoved();
  }

void KeyhoteeMainWindow::setEnabledAttachmentSaveOption( bool enable )
  {
  ui->actionSave_attachement->setEnabled (enable);
  }

void KeyhoteeMainWindow::setEnabledDeleteOption( bool enable )
  {
  ui->actionDelete->setEnabled (enable);
  }

void KeyhoteeMainWindow::setEnabledShareContactOption( bool enable )
  {
  ui->actionShare_contact->setEnabled (enable);
  }

void KeyhoteeMainWindow::refreshMenuOptions() const
{
  bool isContactTableSelected = ui->contacts_page->isSelection();
  bool isContactTreeItemSelected = false;

  if (ui->side_bar->selectedItems().size())
  {
    QTreeWidgetItem* selectedItem = ui->side_bar->selectedItems().first();
    isContactTreeItemSelected =  (selectedItem->type() == ContactItem);
  }

  bool enabled = isContactTableSelected || isContactTreeItemSelected;

  ui->actionCopy->setEnabled (isContactTableSelected);  
  ui->actionCut->setEnabled (false);    
  ui->actionShare_contact->setEnabled (isContactTableSelected);
  ui->actionDelete->setEnabled (enabled);
}

void KeyhoteeMainWindow::setEnabledMailActions(bool enable)
  {
    ui->actionReply->setEnabled(enable);
    ui->actionReply_all->setEnabled(enable);
    ui->actionForward->setEnabled(enable);
  }


void KeyhoteeMainWindow::onRemoveContact()
{  
  QList<QTreeWidgetItem*> selected_items = ui->side_bar->selectedItems();

  if (selected_items.size() && 
      ( ! ui->contacts_page->hasFocusContacts() || ! ui->contacts_page->isSelection() ) )
  {
    QTreeWidgetItem* selectedItem = selected_items.first();
    if (selectedItem->type() == ContactItem)
    { 
      //Find next contact item
      //QTreeWidgetItem* itemNext = ui->side_bar->itemBelow (selectedItem);
      //if (itemNext == nullptr)
      //  itemNext = ui->side_bar->itemAbove (selectedItem);

      onItemContactRemoved (*selectedItem);
      //select next contact item
      //ui->side_bar->setItemSelected (itemNext, true);
    }
  }
  else
    ui->contacts_page->onDeleteContact();

  refreshMenuOptions();
  if(isIdentityPresent() == false ){
       ui->actionNew_Message->setEnabled(false);
       ui->actionRequest_authorization->setEnabled(false);
       if(nullptr != _currentMailbox)
         _currentMailbox->checksendmailbuttons();
       emit checkSendMailSignal();
  }
}

void KeyhoteeMainWindow::keyPressEvent(QKeyEvent *key_event)
{
  if (key_event->key() == Qt::Key_Escape)
  {
    if (ui->widget_stack->currentWidget () == ui->contacts_page)
    {
      if (ui->contacts_page->EscapeIfEditMode())
        return;
    }
  }

  return ATopLevelWindowsContainer::keyPressEvent(key_event);
}

void KeyhoteeMainWindow::onClipboardChanged()
{
#ifndef QT_NO_CLIPBOARD
  if(const QMimeData *md = QApplication::clipboard()->mimeData())
    ui->actionPaste->setEnabled(md->hasText());
#endif
}

void KeyhoteeMainWindow::onFocusChanged(QWidget *old, QWidget *now)
{
  menuEdit->setEnabled(old, now);
}

ContactsTable* KeyhoteeMainWindow::getContactsPage()
{
    return ui->contacts_page;
}

void KeyhoteeMainWindow::onShareContact()
{
  QList<const Contact*> contacts;
  ui->contacts_page->getSelectedContacts(contacts);
  assert(contacts.size());
  shareContact(contacts);
}
