#include "ui_KeyhoteeMainWindow.h"
#include "EditContactDialog.hpp"
#include "KeyhoteeMainWindow.hpp"
#include <bts/application.hpp>

#include <fc/reflect/variant.hpp>
#include <fc/log/logger.hpp>

enum TopLevelItemIndexes
{
    Identities,
    Space1,
    Mailboxes,
    Space2,
    Contacts
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

    connect( ui->actionNew_Contact, &QAction::triggered, this, &KeyhoteeMainWindow::addContact );

    ui->side_bar->topLevelItem(TopLevelItemIndexes::Mailboxes)->setExpanded(true);
    auto space1     = ui->side_bar->topLevelItem(TopLevelItemIndexes::Space1);
    auto space2     = ui->side_bar->topLevelItem(TopLevelItemIndexes::Space2);
    auto space_flags = space1->flags() & (~ Qt::ItemFlags(Qt::ItemIsSelectable) );
    space_flags |= Qt::ItemNeverHasChildren;
    space1->setFlags( space_flags );
    space2->setFlags( space_flags );

    auto contact_root = ui->side_bar->topLevelItem(TopLevelItemIndexes::Contacts);
    contact_root->setExpanded(true);

    auto ident_root = ui->side_bar->topLevelItem(TopLevelItemIndexes::Identities);
    ident_root->setExpanded(true);

    auto app    = bts::application::instance();
    auto pro    = app->get_profile();
    auto idents = pro->identities();
    wlog( "idents: ${idents}", ("idents",idents) );

    for( size_t i = 0; i < idents.size(); ++i )
    {
        auto new_ident_item = new QTreeWidgetItem(ident_root, (QTreeWidgetItem::ItemType)IdentityItem );

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
    auto abook  = pro->get_addressbook();
    auto contacts = abook->get_known_bitnames();
    for( auto itr = contacts.begin(); itr != contacts.end(); ++itr )
    {
        auto new_contact_item = new QTreeWidgetItem(contact_root, (QTreeWidgetItem::ItemType)IdentityItem );

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
}

KeyhoteeMainWindow::~KeyhoteeMainWindow()
{
}

void KeyhoteeMainWindow::addContact()
{
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
}
