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

    auto ident_root = ui->side_bar->topLevelItem(TopLevelItemIndexes::Identities);
    ident_root->setExpanded(true);

    auto pro    = bts::application::instance()->get_profile();
    auto idents = pro->identities();
    wlog( "idents: ${idents}", ("idents",idents) );

    for( size_t i = 0; i < idents.size(); ++i )
    {
        auto new_ident_item = new QTreeWidgetItem(ident_root, (QTreeWidgetItem::ItemType)IdentityItem );
        new_ident_item->setText( 0, idents[i].bit_id.c_str() );
    }
}

KeyhoteeMainWindow::~KeyhoteeMainWindow()
{
}

void KeyhoteeMainWindow::addContact()
{
   EditContactDialog* editcon = new EditContactDialog(this);
   editcon->show();
}
