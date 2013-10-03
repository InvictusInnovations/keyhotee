#include "ui_KeyhoteeMainWindow.h"
#include "KeyhoteeMainWindow.hpp"
#include "AddressBook/AddressBookModel.hpp"
#include "AddressBook/ContactView.hpp"
#include "Mail/MailEditor.hpp"
#include "Mail/InboxModel.hpp"
#include <bts/application.hpp>
#include <bts/bitchat/bitchat_private_message.hpp>

#ifdef Q_OS_MAC
#include <qmacnativetoolbar.h>
#endif

#include <fc/reflect/variant.hpp>
#include <fc/log/logger.hpp>

#include <QLineEdit>
#include <QCompleter>

KeyhoteeMainWindow* GetKeyhoteeWindow()
{
    static KeyhoteeMainWindow* keyhoteeMainWindow = 0;
    if (!keyhoteeMainWindow)
        keyhoteeMainWindow = new KeyhoteeMainWindow;
    return keyhoteeMainWindow;
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
enum MailboxChildren
{
    Inbox,
    Drafts,
    Sent
};
enum SidebarItemTypes
{
    IdentityItem = 2,
    MailboxItem  = 3,
    ContactItem  = 4
};

void ContactGui::setUnreadMsgCount(unsigned int count)
{
    _unread_msg_count = count;
    updateTreeItemDisplay();
}

bool ContactGui::isChatVisible() 
{ 
    return GetKeyhoteeWindow()->isSelectedContactGui(this) && _view->isChatSelected(); 
}

void ContactGui::receiveChatMessage( const QString& from, const QString& msg, const QDateTime& dateTime)
{
    _view->appendChatMessage(from,msg,dateTime);
    if (!isChatVisible())
      {
      setUnreadMsgCount(_unread_msg_count+1);
      }
}

void ContactGui::updateTreeItemDisplay()
{
    QString displayText;
    QString name = _view->getContact().getLabel();
    if (_unread_msg_count)
      displayText = QString("%1 (%2)").arg(name).arg(_unread_msg_count);
    else
      displayText = name;
    _tree_item->setText(0,displayText);
}


class ApplicationDelegate : public bts::application_delegate
{
    public:
     KeyhoteeMainWindow& _mainwindow;
     ApplicationDelegate( KeyhoteeMainWindow& window )
     :_mainwindow(window)
     {
     }

     virtual void received_text( const bts::bitchat::decrypted_message& msg)
     {
        auto opt_contact = _mainwindow._addressbook->get_contact_by_public_key( *(msg.from_key) );
        if( !opt_contact )
        {
            elog( "Recieved text from unknown contact!" );
        }
        else
        {
            wlog( "Received text from known contact!" );
            auto contact_gui = _mainwindow.createContactGuiIfNecessary( opt_contact->wallet_index );
            auto text = msg.as<bts::bitchat::private_text_message>();
            QDateTime dateTime;
            dateTime.setTime_t(msg.sig_time.sec_since_epoch());
            contact_gui->receiveChatMessage( opt_contact->dac_id_string.c_str(), text.msg.c_str(), dateTime );
        }
     }

     virtual void received_email( const bts::bitchat::decrypted_message& msg)
     {
     }
};

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

KeyhoteeMainWindow::KeyhoteeMainWindow()
:QMainWindow()
{
    _app_delegate.reset( new ApplicationDelegate(*this) );
    ui.reset( new Ui::KeyhoteeMainWindow() );
    ui->setupUi(this);
    setWindowIcon( QIcon( ":/images/shield1024.png" ) );

    connect( ui->contacts_page, &ContactsTable::contactOpened, this, &KeyhoteeMainWindow::openContactGui );

    _contact_completer = new QCompleter(this);
    _contact_completer->setModel( modelFromFile( "words.txt", _contact_completer) );
    _contact_completer->setModelSorting( QCompleter::CaseInsensitivelySortedModel );
    _contact_completer->setCaseSensitivity( Qt::CaseInsensitive);
    _contact_completer->setWrapAround(true);

#ifdef Q_OS_MAC
    //QMacNativeToolBar* native_toolbar = QtMacExtras::setNativeToolBar(ui->toolbar, true);
    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
    ui->side_bar->setAttribute(Qt::WA_MacShowFocusRect, 0);
    QApplication::setWindowIcon( QIcon( ":/images/shield1024.icns" ) );
#else
    QApplication::setWindowIcon( QIcon( ":/images/shield1024.png" ) );
#endif

    QWidget* empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    ui->toolbar->addWidget(empty);

    auto search_edit = new QLineEdit( ui->toolbar );
    ui->toolbar->addWidget(search_edit);
    search_edit->setMaximumSize( QSize(  150, 22 ) );
    search_edit->setAttribute(Qt::WA_MacShowFocusRect, 0);
    const char* search_style = "QLineEdit { "\
       "padding-right: 20px; "\
       "padding-left: 5px; "\
       "background: url(:/images/search24x16.png);"\
       "background-position: right;"\
       "background-repeat: no-repeat;"\
       "border: 1px solid gray;"\
       "border-radius: 10px;}";
    search_edit->setStyleSheet( search_style );
    search_edit->setPlaceholderText( tr("Search") );

    QWidget* empty2 = new QWidget();
    empty->resize( QSize(10,10) );
    ui->toolbar->addWidget(empty2);
    
    connect( ui->actionExit, &QAction::triggered, this, &KeyhoteeMainWindow::on_actionExit_triggered );
    connect( ui->actionNew_Message, &QAction::triggered, this, &KeyhoteeMainWindow::newMessage );
    connect( ui->actionEnable_Mining, &QAction::toggled, this, &KeyhoteeMainWindow::enableMining_toggled );    
    connect( ui->actionNew_Contact, &QAction::triggered, this, &KeyhoteeMainWindow::addContact );
    connect( ui->actionShow_Contacts, &QAction::triggered, this, &KeyhoteeMainWindow::showContacts );
    connect( ui->splitter, &QSplitter::splitterMoved, this, &KeyhoteeMainWindow::sideBarSplitterMoved );
    connect( ui->side_bar, &QTreeWidget::itemSelectionChanged, this, &KeyhoteeMainWindow::onSidebarSelectionChanged );

    auto space2     = ui->side_bar->topLevelItem(TopLevelItemIndexes::Space2);
    auto space3     = ui->side_bar->topLevelItem(TopLevelItemIndexes::Space3);
    auto space_flags = space2->flags() & (~ Qt::ItemFlags(Qt::ItemIsSelectable) );
    space_flags |= Qt::ItemNeverHasChildren;
    space2->setFlags( space_flags );
    space3->setFlags( space_flags );

    //_identities_root = ui->side_bar->topLevelItem(TopLevelItemIndexes::Identities);
    _mailboxes_root = ui->side_bar->topLevelItem(TopLevelItemIndexes::Mailboxes);
    _contacts_root = ui->side_bar->topLevelItem(TopLevelItemIndexes::Contacts);
    _wallets_root = ui->side_bar->topLevelItem(TopLevelItemIndexes::Wallets);

    _contacts_root->setExpanded(true);
    //_identities_root->setExpanded(true);
    _mailboxes_root->setExpanded(true);
    _inbox_root   = _mailboxes_root->child(Inbox);
    _drafts_root  = _mailboxes_root->child(Drafts);
    _sent_root    = _mailboxes_root->child(Sent);

    _wallets_root->setExpanded(true);

    auto app    = bts::application::instance();
    app->set_application_delegate( _app_delegate.get() );
    auto pro    = app->get_profile();
    auto idents = pro->identities();

    _inbox  = new InboxModel(this,pro);

    _addressbook_model  = new AddressBookModel( this, pro->get_addressbook() );
    connect( _addressbook_model, &QAbstractItemModel::dataChanged, this, &KeyhoteeMainWindow::addressBookDataChanged );

    ui->contacts_page->setAddressBook(_addressbook_model);
    ui->new_contact->setAddressBook(_addressbook_model);
    ui->inbox_page->setModel(_inbox, MailInbox::Inbox);
    ui->draft_box_page->setModel(_inbox, MailInbox::Drafts);
    ui->sent_box_page->setModel(_inbox, MailInbox::Sent);


    ui->actionEnable_Mining->setChecked(app->get_mining_intensity() != 0);
    wlog( "idents: ${idents}", ("idents",idents) );
    for( size_t i = 0; i < idents.size(); ++i )
    {
    /*
        auto new_ident_item = new QTreeWidgetItem(_identities_root, (QTreeWidgetItem::ItemType)IdentityItem );

        auto id_rec = app->lookup_name( idents[i].bit_id );
        if( !id_rec )
        {
           new_ident_item->setText( 0, (idents[i].bit_id + " [pending]").c_str() );
        }
        else
        {
           new_ident_item->setText( 0, (idents[i].bit_id + " [" + std::to_string(id_rec->repute)+"]" ).c_str() );
        }
    */
        app->mine_name( idents[i].dac_id, 
                        pro->get_keychain().get_identity_key( idents[i].dac_id ).get_public_key(), 
                        idents[i].mining_effort );
    }
    _addressbook = pro->get_addressbook();

    /*
    auto abook  = pro->get_addressbook();
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

}

KeyhoteeMainWindow::~KeyhoteeMainWindow()
{
}

void KeyhoteeMainWindow::addContact()
{
   /*
   EditContactDialog* editcon = new EditContactDialog(this);
   editcon->show();
   connect( editcon, &QDialog::finished, [=]( int result ){
        if( result == QDialog::Accepted )
        {
           auto app    = bts::application::instance();
           auto pro    = app->get_profile();
           auto abook  = pro->get_addressbook();
           abook->store_contact( editcon->getContact() );
        }
        editcon->deleteLater();
     });
   */
  ui->new_contact->setContact( Contact(), ContactView::edit );
  ui->widget_stack->setCurrentWidget( ui->new_contact );
}

void KeyhoteeMainWindow::sideBarSplitterMoved( int pos, int index )
{
    if( pos <= 5 )
    {
        ui->splitter->setHandleWidth(5);
    }
    else
    {
        ui->splitter->setHandleWidth(0);
    }
}

void KeyhoteeMainWindow::addressBookDataChanged( const QModelIndex& top_left, const QModelIndex& bottom_right, const QVector<int>& roles )
{
   const Contact& changed_contact = _addressbook_model->getContact(top_left);
   auto itr = _contact_guis.find( changed_contact.wallet_index );
   if( itr != _contact_guis.end() )
   {
        itr->second.updateTreeItemDisplay();
   }
}

bool KeyhoteeMainWindow::isSelectedContactGui(ContactGui* contactGui)
{
   QList<QTreeWidgetItem*> selected_items = ui->side_bar->selectedItems();
   if ( selected_items.size() == 1)
   {
    return selected_items[0] == contactGui->_tree_item;
   }
   return false;
}

void KeyhoteeMainWindow::onSidebarSelectionChanged()
{
   QList<QTreeWidgetItem*> selected_items = ui->side_bar->selectedItems();
   if( selected_items.size() )
   {
      if( selected_items[0]->type() == ContactItem )
      {
          auto con_id = selected_items[0]->data(0, ContactIdRole ).toInt();
          openContactGui(con_id);
      }
      else if( selected_items[0]->type() == IdentityItem )
      {
          selectIdentityItem( selected_items[0] );
      }
      else if( selected_items[0] == _contacts_root )
      {
          showContacts();
      }
      else if( selected_items[0] == _mailboxes_root )
      {
      }
      /*
      else if( selected_items[0] == _identities_root )
      {
      }
      */
      else if( selected_items[0] == _inbox_root )
      {
          ui->widget_stack->setCurrentWidget( ui->inbox_page );
      }
      else if( selected_items[0] == _drafts_root )
      {
          ui->widget_stack->setCurrentWidget( ui->draft_box_page );
      }
      else if( selected_items[0] == _sent_root )
      {
          ui->widget_stack->setCurrentWidget( ui->sent_box_page );
      }
   }
}

void KeyhoteeMainWindow::selectContactItem( QTreeWidgetItem* item )
{
}

void KeyhoteeMainWindow::selectIdentityItem( QTreeWidgetItem* item )
{
}

void KeyhoteeMainWindow::on_actionExit_triggered()
{
    qApp->closeAllWindows();
}


void KeyhoteeMainWindow::enableMining_toggled(bool enabled)
{
    auto app    = bts::application::instance();
    app->set_mining_intensity(enabled ? 100 : 0);
}

void KeyhoteeMainWindow::showContacts()
{
  ui->side_bar->setCurrentItem( _contacts_root );
  ui->widget_stack->setCurrentWidget( ui->contacts_page );
}

void KeyhoteeMainWindow::newMessage()
{
  auto msg_window = new MailEditor(this, _contact_completer);
  msg_window->show();
}

ContactGui* KeyhoteeMainWindow::getContactGui( int contact_id )
{
   auto itr = _contact_guis.find(contact_id);
   if( itr != _contact_guis.end() )
   {
       return &(itr->second);
   }
   return nullptr;
}

void KeyhoteeMainWindow::openContactGui( int contact_id )
{
    if( contact_id == -1 ) // TODO: define -1 as AddressBookID
    {
        showContacts();
        return;
    }
    else
    {
        auto contact_gui = createContactGuiIfNecessary( contact_id );
        showContactGui( *contact_gui );
    }
}


ContactGui* KeyhoteeMainWindow::createContactGuiIfNecessary( int contact_id)
{
    ContactGui* contact_gui = getContactGui(contact_id);
    if (!contact_gui)
    {
        createContactGui(contact_id);
        contact_gui = getContactGui(contact_id);
    }
    return contact_gui;
}

void KeyhoteeMainWindow::createContactGui( int contact_id )
{
    //DLNFIX2 maybe cleanup/refactor ContactGui construction later
    auto new_contact_item = new QTreeWidgetItem(_contacts_root, 
                                                (QTreeWidgetItem::ItemType)ContactItem );
    new_contact_item->setData( 0, ContactIdRole, contact_id );

    auto view = new ContactView( ui->widget_stack );

    //add new contactGui to map
    ContactGui contact_gui(new_contact_item,view);
    _contact_guis[contact_id] = contact_gui;

    view->setAddressBook( _addressbook_model );
    const Contact& con = _addressbook_model->getContactById( contact_id );
    view->setContact(con);
    ui->widget_stack->addWidget( view );

}

void KeyhoteeMainWindow::showContactGui( ContactGui& contact_gui )
{
    ui->side_bar->setCurrentItem( contact_gui._tree_item );
    ui->widget_stack->setCurrentWidget( contact_gui._view );
    if (contact_gui.isChatVisible())
    {
    contact_gui._view->onChat();
    }
}


