#include "ContactsTable.hpp"
#include "ui_ContactsTable.h"
#include "AddressBookModel.hpp"
#include <bts/application.hpp>
#include <bts/profile.hpp>
#include "AddressBook/ContactView.hpp"

#include <QSortFilterProxyModel>
#include <QHeaderView>

#include <QMessageBox>

class ContactsSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    ContactsSortFilterProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent) {}
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

bool ContactsSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex first_name_index = sourceModel()->index(sourceRow, AddressBookModel::FirstName, sourceParent);
    QModelIndex last_name_index  = sourceModel()->index(sourceRow, AddressBookModel::LastName, sourceParent);
    QModelIndex id_index         = sourceModel()->index(sourceRow, AddressBookModel::Id, sourceParent);
    return (sourceModel()->data(first_name_index).toString().contains(filterRegExp()) ||
            sourceModel()->data(last_name_index).toString().contains(filterRegExp())  ||
            sourceModel()->data(id_index).toString().contains(filterRegExp())           );
}

void ContactsTable::searchEditChanged(QString search_string)
{
   QSortFilterProxyModel* model = dynamic_cast<QSortFilterProxyModel*>(ui->contact_table->model());
   QRegExp regex(search_string, Qt::CaseInsensitive, QRegExp::FixedString);
   model->setFilterRegExp(regex);
}

ContactsTable::ContactsTable( QWidget* parent )
: QWidget(parent), 
  ui( new Ui::ContactsTable() )
{
  ui->setupUi(this); 
  //_delete_contact = new QAction(this); //QIcon( ":/images/delete_icon.png"), tr( "Delete" ), this);
  //_delete_contact->setShortcut(Qt::Key_Delete);
  //addAction(_delete_contact);

  //connect( _delete_contact, &QAction::triggered, this, &ContactsTable::onDeleteContact);
}

ContactsTable::~ContactsTable(){}

void ContactsTable::setAddressBook( AddressBookModel* addressbook_model )
{
  _addressbook_model = addressbook_model;
  if( _addressbook_model )
  {
     _sorted_addressbook_model = new ContactsSortFilterProxyModel( this );
     _sorted_addressbook_model->setSourceModel( _addressbook_model );
     _sorted_addressbook_model->setDynamicSortFilter(true);
     ui->contact_table->setModel( _sorted_addressbook_model );
  }
  ui->contact_table->setShowGrid(false);
  ui->contact_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->contact_table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  connect( ui->contact_table, &QAbstractItemView::clicked, this, &ContactsTable::openContact );
}

void ContactsTable::openContact( const QModelIndex& index )
{
   QModelIndex mapped_index = _sorted_addressbook_model->mapToSource(index);
   auto contact_id = _addressbook_model->getContact(mapped_index).wallet_index;
   Q_EMIT contactOpened( contact_id );
}

void ContactsTable::onDeleteContact()
{
   //remove selected contacts from inbox model (and database)
   QSortFilterProxyModel* model = dynamic_cast<QSortFilterProxyModel*>(ui->contact_table->model());
   //model->setUpdatesEnabled(false);
   QItemSelectionModel* selection_model = ui->contact_table->selectionModel();
   QModelIndexList sortFilterIndexes = selection_model->selectedRows();
   QModelIndexList indexes;
   foreach(QModelIndex sortFilterIndex,sortFilterIndexes)
     indexes.append(model->mapToSource(sortFilterIndex));
   qSort(indexes);
   if (indexes.count() == 0)
     return;
   if(QMessageBox::question(this, "Delete Contact", "Are you sure you want to delete this contact?") == QMessageBox::Button::No)
     return;
   auto sourceModel = model->sourceModel();
   for(int i = indexes.count() - 1; i > -1; --i)
       sourceModel->removeRows(indexes.at(i).row(),1);
   //model->setUpdatesEnabled(true);   

   //TODO Remove fullname/bitname for deleted contacts from QCompleter
}

bool ContactsTable::isShowDetailsHidden()
{
  return ui->contact_details_view->isHidden();
}

void ContactsTable::on_actionShow_details_toggled(bool checked)
{
  if (checked)
  {
    ui->contact_details_view->show();
  }
  else
  {
    ui->contact_details_view->hide();
  }
  
}

void ContactsTable::addContactView( ContactView& view ) const {
   ui->contact_details_view->addWidget (&view);
}

void ContactsTable::showView( ContactView& view ) const {
   view.initTab ();   
   ui->contact_details_view->setCurrentWidget (&view);   
}