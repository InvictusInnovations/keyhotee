#include "ContactsTable.hpp"
#include "ui_ContactsTable.h"
#include "AddressBookModel.hpp"
#include <bts/application.hpp>
#include <bts/profile.hpp>

#include <QSortFilterProxyModel>
#include <QHeaderView>

ContactsTable::ContactsTable( QWidget* parent )
:QWidget(parent), ui( new Ui::ContactsTable() )
{
  ui->setupUi(this); 
}


ContactsTable::~ContactsTable(){}

void ContactsTable::setAddressBook( AddressBookModel* abook_model )
{
  _addressbook_model = abook_model;
  if( _addressbook_model )
  {
     _sorted_addressbook_model = new QSortFilterProxyModel( this );
     _sorted_addressbook_model->setSourceModel( _addressbook_model );
     _sorted_addressbook_model->setDynamicSortFilter(true);
     ui->contact_table->setModel( _sorted_addressbook_model );
  }
  ui->contact_table->setShowGrid(false);
  ui->contact_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->contact_table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  connect( ui->contact_table, &QAbstractItemView::activated, 
           this, &ContactsTable::openContact );
}
void ContactsTable::openContact( const QModelIndex& index )
{
   auto contact_id = _addressbook_model->getContact(index).wallet_account_index;

   Q_EMIT contactOpened( contact_id );
}
