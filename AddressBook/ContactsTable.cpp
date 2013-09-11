#include "ContactsTable.hpp"
#include "ui_ContactsTable.h"
#include "AddressBookModel.hpp"
#include <bts/application.hpp>
#include <bts/profile.hpp>

#include <QSortFilterProxyModel>

ContactsTable::ContactsTable( QWidget* parent )
:QWidget(parent), ui( new Ui::ContactsTable() )
{
  ui->setupUi(this); 
  
  _addressbook_model        = new AddressBookModel( this, bts::application::instance()->get_profile()->get_addressbook() );
  _sorted_addressbook_model = new QSortFilterProxyModel( this );
  _sorted_addressbook_model->setSourceModel( _addressbook_model );
  _sorted_addressbook_model->setDynamicSortFilter(true);
  ui->contact_table->setModel( _sorted_addressbook_model );
}

ContactsTable::~ContactsTable(){}
