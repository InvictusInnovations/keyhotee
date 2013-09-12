#include "ui_KeyhoteeMainWindow.h"
#include "EditContactDialog.hpp"
#include "KeyhoteeMainWindow.hpp"
#include <bts/application.hpp>
#include <QLineEdit>

#include <fc/reflect/variant.hpp>
#include <fc/log/logger.hpp>

enum TopLevelItemIndexes
{
    Mailboxes,
    Space2,
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

KeyhoteeMainWindow::KeyhoteeMainWindow()
:QMainWindow()
{
    ui.reset( new Ui::KeyhoteeMainWindow() );
    ui->setupUi(this);

#ifdef Q_OS_MAC
    setUnifiedTitleAndToolBarOnMac(true);
    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
    ui->side_bar->setAttribute(Qt::WA_MacShowFocusRect, 0);
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
    

    connect( ui->actionNew_Contact, &QAction::triggered, this, &KeyhoteeMainWindow::addContact );
    connect( ui->actionShow_Contacts, &QAction::triggered, this, &KeyhoteeMainWindow::showContacts );

    auto space2     = ui->side_bar->topLevelItem(TopLevelItemIndexes::Space2);
    auto space_flags = space2->flags() & (~ Qt::ItemFlags(Qt::ItemIsSelectable) );
    space_flags |= Qt::ItemNeverHasChildren;
    space2->setFlags( space_flags );

    //_identities_root = ui->side_bar->topLevelItem(TopLevelItemIndexes::Identities);
    _mailboxes_root = ui->side_bar->topLevelItem(TopLevelItemIndexes::Mailboxes);
    _contacts_root = ui->side_bar->topLevelItem(TopLevelItemIndexes::Contacts);

    _contacts_root->setExpanded(true);
    //_identities_root->setExpanded(true);
    _mailboxes_root->setExpanded(true);
    _inbox_root   = _mailboxes_root->child(Inbox);
    _drafts_root  = _mailboxes_root->child(Drafts);
    _sent_root    = _mailboxes_root->child(Sent);


    auto app    = bts::application::instance();
    auto pro    = app->get_profile();
    auto idents = pro->identities();
    wlog( "idents: ${idents}", ("idents",idents) );

    /*
    for( size_t i = 0; i < idents.size(); ++i )
    {
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

        app->mine_name( idents[i].bit_id, 
                        pro->get_keychain().get_identity_key( idents[i].bit_id ).get_public_key(), 
                        idents[i].mining_effort );
    }
    */
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

    connect( ui->side_bar, &QTreeWidget::itemSelectionChanged, this, &KeyhoteeMainWindow::onSidebarSelectionChanged );
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
  ui->widget_stack->setCurrentWidget( ui->new_contact );
}
void KeyhoteeMainWindow::onSidebarSelectionChanged()
{
   QList<QTreeWidgetItem*> selected_items = ui->side_bar->selectedItems();
   if( selected_items.size() )
   {
      if( selected_items[0]->type() == ContactItem )
      {
          selectContactItem( selected_items[0] );
      }
      else if( selected_items[0]->type() == IdentityItem )
      {
          selectIdentityItem( selected_items[0] );
      }
      else if( selected_items[0] == _contacts_root )
      {

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
      }
      else if( selected_items[0] == _drafts_root )
      {
      }
      else if( selected_items[0] == _sent_root )
      {

      }
   }
}

void KeyhoteeMainWindow::selectContactItem( QTreeWidgetItem* item )
{
}

void KeyhoteeMainWindow::selectIdentityItem( QTreeWidgetItem* item )
{
}

void KeyhoteeMainWindow::showContacts()
{
  ui->widget_stack->setCurrentWidget( ui->contacts_page );
}
