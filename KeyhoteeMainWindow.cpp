#include "KeyhoteeMainWindow.hpp"

#include "ui_KeyhoteeMainWindow.h"
#include "diagnosticdialog.h"
#include "AddressBook/AddressBookModel.hpp"
#include "AddressBook/NewIdentityDialog.hpp"
#include "AddressBook/ContactView.hpp"

#include "Mail/MailboxModel.hpp"
#include "Mail/MailEditor.hpp"
#include "Mail/maileditorwindow.hpp"

#include "connectionstatusframe.h"
#include "GitSHA1.h"
#include "KeyhoteeApplication.hpp"

#include <bts/bitchat/bitchat_private_message.hpp>

#ifdef Q_OS_MAC
//#include <qmacnativetoolbar.h>
#endif

#include <fc/reflect/variant.hpp>
#include <fc/log/logger.hpp>

/// QT headers:
#include <QCompleter>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>

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
  Contacts
};

void ContactGui::setUnreadMsgCount(unsigned int count)
{
  _unread_msg_count = count;
  updateTreeItemDisplay();
}

bool ContactGui::isChatVisible()
{
  return getKeyhoteeWindow()->isSelectedContactGui(this) && _view->isChatSelected();
}

void ContactGui::receiveChatMessage(const QString& from, const QString& msg, const QDateTime& dateTime)
{
  _view->appendChatMessage(from, msg, dateTime);
  if (!isChatVisible())
    setUnreadMsgCount(_unread_msg_count + 1);
}

void ContactGui::updateTreeItemDisplay()
{
  QString display_text;
  QString name = _view->getContact().getLabel();
  if (_unread_msg_count)
    display_text = QString("%1 (%2)").arg(name).arg(_unread_msg_count);
  else
    display_text = name;
  _tree_item->setText(0, display_text);
  _tree_item->setHidden (false);
  }

QAbstractItemModel* modelFromFile(const QString& fileName, QCompleter* completer)
{
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly))
    return new QStringListModel(completer);

#ifndef QT_NO_CURSOR
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
  QStringList words;

  while (!file.atEnd())
  {
    QByteArray line = file.readLine();
    if (!line.isEmpty())
      words << line.trimmed();
  }

#ifndef QT_NO_CURSOR
  QApplication::restoreOverrideCursor();
#endif
  return new QStringListModel(words, completer);
}

KeyhoteeMainWindow::KeyhoteeMainWindow(const TKeyhoteeApplication& mainApp) :
  SelfSizingMainWindow(),
  MailProcessor(*this, bts::application::instance()->get_profile())
{
  ui.reset(new Ui::KeyhoteeMainWindow() );
  ui->setupUi(this);
  setWindowIcon(QIcon(":/images/shield1024.png") );

  QString title = QString("%1 (%2)").arg(mainApp.getAppName().c_str()).arg(mainApp.getLoadedProfileName().c_str());
  setWindowTitle(title);

  connect(ui->contacts_page, &ContactsTable::contactOpened, this, &KeyhoteeMainWindow::openContactGui);
  connect(ui->contacts_page, &ContactsTable::contactDeleted, this, &KeyhoteeMainWindow::deleteContactGui);

#ifdef Q_OS_MAC
  //QMacNativeToolBar* native_toolbar = QtMacExtras::setNativeToolBar(ui->toolbar, true);
  QApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
  ui->side_bar->setAttribute(Qt::WA_MacShowFocusRect, 0);
  QApplication::setWindowIcon(QIcon(":/images/shield1024.icns") );
#else
  QApplication::setWindowIcon(QIcon(":/images/shield1024.png") );
#endif

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

  // ---------------------- MenuBar
  // File
  connect(ui->actionExit, &QAction::triggered, this, &KeyhoteeMainWindow::on_actionExit_triggered);
  // Edit
  connect(ui->actionCopy, &QAction::triggered, this, &KeyhoteeMainWindow::on_actionCopy_triggered);
  connect(ui->actionCut, &QAction::triggered, this, &KeyhoteeMainWindow::on_actionCut_triggered);
  connect(ui->actionPaste, &QAction::triggered, this, &KeyhoteeMainWindow::on_actionPaste_triggered);
  connect(ui->actionSelect_All, &QAction::triggered, this, &KeyhoteeMainWindow::on_actionSelectAll_triggered);
  connect(ui->actionDelete, &QAction::triggered, this, &KeyhoteeMainWindow::on_actionDelete_triggered);
  // Identity
  connect(ui->actionNew_identity, &QAction::triggered, this, &KeyhoteeMainWindow::on_actionNew_identity_triggered);
  connect(ui->actionEnable_Mining, &QAction::toggled, this, &KeyhoteeMainWindow::enableMining_toggled);
  // Mail
  connect(ui->actionNew_Message, &QAction::triggered, this, &KeyhoteeMainWindow::newMailMessage);
  connect(ui->actionReply, &QAction::triggered, this, &KeyhoteeMainWindow::on_actionReply_triggered);
  connect(ui->actionReply_all, &QAction::triggered, this, &KeyhoteeMainWindow::on_actionReply_all_triggered);
  connect(ui->actionForward, &QAction::triggered, this, &KeyhoteeMainWindow::on_actionForward_triggered);
  connect(ui->actionSave_attachement, &QAction::triggered, this, &KeyhoteeMainWindow::on_actionSave_attachement_triggered);
  // Contact
  connect(ui->actionNew_Contact, &QAction::triggered, this, &KeyhoteeMainWindow::addContact);
  connect(ui->actionSet_Icon, &QAction::triggered, this, &KeyhoteeMainWindow::on_actionset_Icon_triggered);
  connect(ui->actionShow_Contacts, &QAction::triggered, this, &KeyhoteeMainWindow::showContacts);
  // Help
  connect(ui->actionDiagnostic, &QAction::triggered, this, &KeyhoteeMainWindow::on_actionDiagnostic_triggered);
  connect(ui->actionAbout, &QAction::triggered, this, &KeyhoteeMainWindow::on_actionAbout_triggered);

  connect(ui->splitter, &QSplitter::splitterMoved, this, &KeyhoteeMainWindow::sideBarSplitterMoved);
  connect(ui->side_bar, &TreeWidgetCustom::itemSelectionChanged, this, &KeyhoteeMainWindow::onSidebarSelectionChanged);
  connect(ui->side_bar, &TreeWidgetCustom::itemDoubleClicked, this, &KeyhoteeMainWindow::onSidebarDoubleClicked);
  connect(ui->side_bar, &TreeWidgetCustom::itemContactRemoved, this, &KeyhoteeMainWindow::onItemContactRemoved);

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

  _contacts_root->setExpanded(true);
  //_identities_root->setExpanded(true);
  _mailboxes_root->setExpanded(true);
  _inbox_root = _mailboxes_root->child(Inbox);
  _drafts_root = _mailboxes_root->child(Drafts);
  _sent_root = _mailboxes_root->child(Sent);

  _wallets_root->setExpanded(true);
  _bitcoin_root = _wallets_root->child(Bitcoin);
  _bitshares_root = _wallets_root->child(BitShares);
  _litecoin_root = _wallets_root->child(Litecoin);

  auto app = bts::application::instance();
  app->set_application_delegate(this);
  auto profile = app->get_profile();
  auto idents = profile->identities();

  auto addressbook = profile->get_addressbook();
  _addressbook_model = new AddressBookModel(this, addressbook);

  _inbox_model = new MailboxModel(this, profile, profile->get_inbox_db(), *_addressbook_model);
  _draft_model = new MailboxModel(this, profile, profile->get_draft_db(), *_addressbook_model);
  _pending_model = new MailboxModel(this, profile, profile->get_pending_db(), *_addressbook_model);
  _sent_model = new MailboxModel(this, profile, profile->get_sent_db(), *_addressbook_model);

  connect(_addressbook_model, &QAbstractItemModel::dataChanged, this,
    &KeyhoteeMainWindow::addressBookDataChanged);

  MailEditor::setContactCompleter(_addressbook_model->getContactCompleter() );

  ui->contacts_page->setAddressBook(_addressbook_model);
  ui->new_contact->setAddressBook(_addressbook_model);
  ui->inbox_page->setModel(MailProcessor, _inbox_model, Mailbox::Inbox);
  ui->draft_box_page->setModel(MailProcessor, _draft_model, Mailbox::Drafts);
  ui->sent_box_page->setModel(MailProcessor, _sent_model, Mailbox::Sent);

  ui->widget_stack->setCurrentWidget(ui->inbox_page);
  connect(ui->actionDelete, SIGNAL(triggered()), ui->inbox_page, SLOT(onDeleteMail()));
  connect(ui->actionShow_details, SIGNAL(toggled(bool)), ui->inbox_page, SLOT(on_actionShow_details_toggled(bool)));

  wlog("idents: ${idents}", ("idents", idents) );
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
  QString settings_file = "keyhotee_";
  settings_file.append(mainApp.getLoadedProfileName().c_str());
  setSettingsFile(settings_file);
  readSettings();
}

KeyhoteeMainWindow::~KeyhoteeMainWindow()
{}

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
    disconnect(ui->actionDelete, SIGNAL(triggered()), ui->contacts_page, SLOT(onDeleteContact()));
    disconnect(ui->actionDelete, SIGNAL(triggered()), ui->inbox_page, SLOT(onDeleteMail()));
    disconnect(ui->actionDelete, SIGNAL(triggered()), ui->draft_box_page, SLOT(onDeleteMail()));
    disconnect(ui->actionDelete, SIGNAL(triggered()), ui->sent_box_page, SLOT(onDeleteMail()));
    disconnect(ui->actionShow_details, SIGNAL(toggled(bool)), ui->contacts_page, SLOT(on_actionShow_details_toggled(bool)));
    disconnect(ui->actionShow_details, SIGNAL(toggled(bool)), ui->inbox_page, SLOT(on_actionShow_details_toggled(bool)));
    disconnect(ui->actionShow_details, SIGNAL(toggled(bool)), ui->draft_box_page, SLOT(on_actionShow_details_toggled(bool)));
    disconnect(ui->actionShow_details, SIGNAL(toggled(bool)), ui->sent_box_page, SLOT(on_actionShow_details_toggled(bool)));

    if (selected_items[0]->type() == ContactItem)
    {
      auto con_id = selected_items[0]->data(0, ContactIdRole).toInt();
      openContactGui(con_id);
      //this makes overstack when contact_page table is sorted, 
      //selectRow generate signal onSidebarSelectionChanged and openContactGui is call two or more
      ui->contacts_page->selectRow(con_id);
      connect(ui->actionDelete, SIGNAL(triggered()), ui->contacts_page, SLOT(onDeleteContact()));
      connect(ui->actionShow_details, SIGNAL(toggled(bool)), ui->contacts_page, SLOT(on_actionShow_details_toggled(bool)));
      if(ui->contacts_page->isShowDetailsHidden())
        ui->actionShow_details->setChecked(false);
      else
        ui->actionShow_details->setChecked(true);
    }
    else if (selected_items[0]->type() == IdentityItem)
    {
      selectIdentityItem(selected_items[0]);
    }
    else if (selected_items[0] == _contacts_root)
    {
      showContacts();
      connect(ui->actionDelete, SIGNAL(triggered()), ui->contacts_page, SLOT(onDeleteContact()));
      connect(ui->actionShow_details, SIGNAL(toggled(bool)), ui->contacts_page, SLOT(on_actionShow_details_toggled(bool)));
      if (ui->contacts_page->isShowDetailsHidden())
        ui->actionShow_details->setChecked(false);
      else
        ui->actionShow_details->setChecked(true);
    }
    else if (selected_items[0] == _mailboxes_root)
    {
      ui->widget_stack->setCurrentWidget(ui->inbox_page);
      connect(ui->actionDelete, SIGNAL(triggered()), ui->inbox_page, SLOT(onDeleteMail()));
      connect(ui->actionShow_details, SIGNAL(toggled(bool)), ui->inbox_page, SLOT(on_actionShow_details_toggled(bool)));
      if (ui->inbox_page->isShowDetailsHidden())
        ui->actionShow_details->setChecked(false);
      else
        ui->actionShow_details->setChecked(true);
    }
    /*
       else if( selected_items[0] == _identities_root )
     {
     }
     */
    else if (selected_items[0] == _inbox_root)
    {
      ui->widget_stack->setCurrentWidget(ui->inbox_page);
      connect(ui->actionDelete, SIGNAL(triggered()), ui->inbox_page, SLOT(onDeleteMail()));
      connect(ui->actionShow_details, SIGNAL(toggled(bool)), ui->inbox_page, SLOT(on_actionShow_details_toggled(bool)));
      if (ui->inbox_page->isShowDetailsHidden())
        ui->actionShow_details->setChecked(false);
      else
        ui->actionShow_details->setChecked(true);
    }
    else if (selected_items[0] == _drafts_root)
    {
      ui->widget_stack->setCurrentWidget(ui->draft_box_page);
      connect(ui->actionDelete, SIGNAL(triggered()), ui->draft_box_page, SLOT(onDeleteMail()));
      connect(ui->actionShow_details, SIGNAL(toggled(bool)), ui->draft_box_page, SLOT(on_actionShow_details_toggled(bool)));
      if (ui->draft_box_page->isShowDetailsHidden())
        ui->actionShow_details->setChecked(false);
      else
        ui->actionShow_details->setChecked(true);
    }
    else if (selected_items[0] == _sent_root)
    {
      ui->widget_stack->setCurrentWidget(ui->sent_box_page);
      connect(ui->actionDelete, SIGNAL(triggered()), ui->sent_box_page, SLOT(onDeleteMail()));
      connect(ui->actionShow_details, SIGNAL(toggled(bool)), ui->sent_box_page, SLOT(on_actionShow_details_toggled(bool)));
      if (ui->sent_box_page->isShowDetailsHidden())
        ui->actionShow_details->setChecked(false);
      else
        ui->actionShow_details->setChecked(true);
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
void KeyhoteeMainWindow::on_actionExit_triggered()
{
  qApp->closeAllWindows();
}

// Menu Edit
void KeyhoteeMainWindow::on_actionCopy_triggered()
{
  notSupported();
}

void KeyhoteeMainWindow::on_actionCut_triggered()
{
  notSupported();
}

void KeyhoteeMainWindow::on_actionPaste_triggered()
{
  notSupported();
}

void KeyhoteeMainWindow::on_actionSelectAll_triggered()
{
  notSupported();
}

void KeyhoteeMainWindow::on_actionDelete_triggered()
{
  // notSupported();
}

// Menu Identity
void KeyhoteeMainWindow::on_actionNew_identity_triggered()
{
   NewIdentityDialog* ident_dialog = new NewIdentityDialog(this);
   ident_dialog->show();
}

void KeyhoteeMainWindow::enableMining_toggled(bool enabled)
{
  auto app = bts::application::instance();
  app->set_mining_intensity(enabled ? 100 : 0);
}

// Menu Mail
void KeyhoteeMainWindow::on_actionReply_triggered()
{
  notSupported();
}

void KeyhoteeMainWindow::on_actionReply_all_triggered()
{
  notSupported();
}

void KeyhoteeMainWindow::on_actionForward_triggered()
{
  notSupported();
}

void KeyhoteeMainWindow::on_actionSave_attachement_triggered()
{
  notSupported();
}

// Menu Contact
void KeyhoteeMainWindow::on_actionset_Icon_triggered()
{
  notSupported();
}

void KeyhoteeMainWindow::displayDiagnosticLog()
{
  DiagnosticDialog diagnoslic_dialog;
  diagnoslic_dialog.setModal(true);
  diagnoslic_dialog.exec();
}
// Menu Help
void KeyhoteeMainWindow::on_actionDiagnostic_triggered()
{
  displayDiagnosticLog();
}

void KeyhoteeMainWindow::on_actionAbout_triggered()
{
  QString title(tr("About "));
  title += windowTitle();
  QString text;
  text = tr("<p align='center'><b>");
  text += windowTitle();
  text += tr(" version ");
  text += tr(APPLICATION_VERSION);
  text += tr("</b><br/><br/>");
  /// Build tag: <a href="https://github.com/InvictusInnovations/keyhotee/commit/xxxx">xxxx</a>
  text += tr("Built from revision: <a href=\"https://github.com/InvictusInnovations/keyhotee/commit/");
  text += tr(g_GIT_SHA1);
  text += tr("\">");
  text += tr(g_GIT_SHA1);
  text += tr("</a>");
  text += tr("<br/>");
  text += tr("Invictus Innovations Inc<br/>");
  text += tr("<a href=\"http://invictus-innovations.com/keyhotee/\">http://invictus-innovations.com/keyhotee/</a>");
  text += tr("<br/></p>");

  QMessageBox::about(this, title, text);
}

void KeyhoteeMainWindow::showContacts()
{
  ui->side_bar->setCurrentItem(_contacts_root);
  ui->widget_stack->setCurrentWidget(ui->contacts_page);
}

void KeyhoteeMainWindow::newMailMessage()
{
  MailEditorMainWindow* mailWindow = new MailEditorMainWindow(this, *_addressbook_model,
    MailProcessor, true);
  mailWindow->show();

  //auto msg_window = new MailEditor(this);
  //msg_window->addToContact(-1);
  //msg_window->setFocusAndShow();
}

void KeyhoteeMainWindow::newMailMessageTo(const Contact& contact)
{
  MailEditorMainWindow* mailWindow = new MailEditorMainWindow(this, *_addressbook_model,
    MailProcessor, true);

  IMailProcessor::TRecipientPublicKeys toList, emptyList;
  toList.push_back(contact.public_key);
  mailWindow->SetRecipientList(toList, emptyList, emptyList);
  mailWindow->show();

  //auto msg_window = new MailEditor(this);
  //msg_window->addToContact(contact_id);
  //msg_window->setFocusAndShow();
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
  return contact_gui;
}

void KeyhoteeMainWindow::createContactGui(int contact_id)
{
  //DLNFIX2 maybe cleanup/refactor ContactGui construction later
  auto new_contact_item = new QTreeWidgetItem(_contacts_root,
                                              (QTreeWidgetItem::ItemType)ContactItem);
  new_contact_item->setData(0, ContactIdRole, contact_id);

  auto       view = new ContactView(ui->widget_stack);

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
    _contacts_root->removeChild(contact_gui->_tree_item);
    _contact_guis.erase(contact_id);
}

void KeyhoteeMainWindow::setupStatusBar()
{
  QStatusBar*             sb = statusBar();
  TConnectionStatusFrame* cs = new TConnectionStatusFrame(ConnectionStatusDS);
  sb->addPermanentWidget(cs);
}

void KeyhoteeMainWindow::received_text(const bts::bitchat::decrypted_message& msg)
{
  auto opt_contact = _addressbook->get_contact_by_public_key(*(msg.from_key) );
  if (!opt_contact)
  {
    elog("Received text from unknown contact!");
  }
  else
  {
    wlog("Received text from known contact!");
    auto      contact_gui = createContactGuiIfNecessary(opt_contact->wallet_index);
    auto      text = msg.as<bts::bitchat::private_text_message>();
    QDateTime dateTime;
    dateTime.setTime_t(msg.sig_time.sec_since_epoch());
    bts::get_profile()->get_chat_db()->store(msg);
    contact_gui->receiveChatMessage(opt_contact->dac_id_string.c_str(), text.msg.c_str(), dateTime);
  }
}

void KeyhoteeMainWindow::received_email(const bts::bitchat::decrypted_message& msg)
{
  auto header = bts::get_profile()->get_inbox_db()->store(msg);
  _inbox_model->addMailHeader(header);
}

void KeyhoteeMainWindow::OnMessageSaving()
{
  /// FIXME - add some status bar messaging
}

void KeyhoteeMainWindow::OnMessageSaved(const TStoredMailMessage& msg,
                                        const TStoredMailMessage* overwrittenOne) 
{
  /// FIXME - add some status bar messaging
  if(overwrittenOne != nullptr)
    _draft_model->replaceMessage(*overwrittenOne, msg);
  else
    _draft_model->addMailHeader(msg);

  ui->draft_box_page->refreshMessageViewer();
}

void KeyhoteeMainWindow::OnMessageGroupPending(unsigned int count)
{
  /// FIXME - add some status bar messaging
}

void KeyhoteeMainWindow::OnMessagePending(const TStoredMailMessage& msg)
{
  /// FIXME - add some status bar messaging
}

void KeyhoteeMainWindow::OnMessageGroupPendingEnd()
{
  /// FIXME - add some status bar messaging
}

void KeyhoteeMainWindow::OnMessageSendingStart()
{
  /// FIXME - add some status bar messaging
}

void KeyhoteeMainWindow::OnMessageSent(const TStoredMailMessage& pendingMsg,
  const TStoredMailMessage& sentMsg)
{
  /// FIXME - add some status bar messaging
}

void KeyhoteeMainWindow::OnMessageSendingEnd()
{
  /// FIXME - add some status bar messaging
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

void KeyhoteeMainWindow::onSavedNewContact()
{
  enableMenu(true);
  //Issue #43
  //After saving just added contact, contact detail window should point to newly added record
  ui->contacts_page->selectRow(0);
}

void KeyhoteeMainWindow::enableMenu(bool enable)
{
  ui->actionShow_details->setEnabled (enable);
  ui->actionDelete->setEnabled (enable);
  ui->actionNew_Contact->setEnabled (enable);
  ui->actionShow_Contacts->setEnabled (enable);
  _search_edit->setEnabled (enable);  
}

void KeyhoteeMainWindow::closeEvent(QCloseEvent *closeEvent)
{
  if (checkSaving())
    closeEvent->accept();
  else
    closeEvent->ignore();
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