#include "ContactsTable.hpp"
#include "ui_ContactsTable.h"
#include "AddressBookModel.hpp"
#include <bts/application.hpp>
#include <bts/profile.hpp>
#include "AddressBook/ContactView.hpp"
#include "KeyhoteeMainWindow.hpp"

#include <QClipboard>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QMessageBox>

class ContactsSortFilterProxyModel : public QSortFilterProxyModel
{
public:
  ContactsSortFilterProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent)
    {
    setSortRole(Qt::UserRole);
    }

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

bool ContactsSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
  {
  QModelIndex first_name_index = sourceModel()->index(sourceRow, AddressBookModel::FirstName, sourceParent);
  QModelIndex last_name_index = sourceModel()->index(sourceRow, AddressBookModel::LastName, sourceParent);
  QModelIndex id_index = sourceModel()->index(sourceRow, AddressBookModel::Id, sourceParent);
  return sourceModel()->data(first_name_index).toString().contains(filterRegExp()) ||
         sourceModel()->data(last_name_index).toString().contains(filterRegExp()) ||
         sourceModel()->data(id_index).toString().contains(filterRegExp());
  }

void ContactsTable::searchEditChanged(QString search_string)
  {
  QSortFilterProxyModel* model = dynamic_cast<QSortFilterProxyModel*>(ui->contact_table->model());
  QRegExp                regex(search_string, Qt::CaseInsensitive, QRegExp::FixedString);
  model->setFilterRegExp(regex);
  }

ContactsTable::ContactsTable(QWidget* parent)
  : QWidget(parent),
  ui(new Ui::ContactsTable() ),
  _currentWidgetView (nullptr)
  {
  ui->setupUi(this);
  ui->contact_table->setModificationsChecker (this);
  //_delete_contact = new QAction(this); //QIcon( ":/images/delete_icon.png"), tr( "Delete" ), this);
  //_delete_contact->setShortcut(Qt::Key_Delete);
  //addAction(_delete_contact);

  //connect( _delete_contact, &QAction::triggered, this, &ContactsTable::onDeleteContact);
  }

ContactsTable::~ContactsTable(){}

void ContactsTable::setAddressBook(AddressBookModel* addressbook_model)
  {
  _addressbook_model = addressbook_model;
  if (_addressbook_model)
    {
    _sorted_addressbook_model = new ContactsSortFilterProxyModel(this);
    _sorted_addressbook_model->setSourceModel(_addressbook_model);
    _sorted_addressbook_model->setDynamicSortFilter(true);
    ui->contact_table->setModel(_sorted_addressbook_model);
    }
  ui->contact_table->setShowGrid(false);
  ui->contact_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
  ui->contact_table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  QItemSelectionModel* selection_model = ui->contact_table->selectionModel();
  connect(selection_model, &QItemSelectionModel::selectionChanged, this, &ContactsTable::onSelectionChanged);
  }

void ContactsTable::onSelectionChanged (const QItemSelection &selected, const QItemSelection &deselected)
  {
  QItemSelectionModel* selection_model = ui->contact_table->selectionModel();
  QModelIndexList      indexes = selection_model->selectedRows();
  bool                 oneRow = (indexes.size() == 1);

  if (oneRow)
    {
    QModelIndex mapped_index = _sorted_addressbook_model->mapToSource(indexes[0]);
    auto        contact_id = _addressbook_model->getContact(mapped_index).wallet_index;
    Q_EMIT      contactOpened(contact_id);      
    }
  else
    {
    _currentWidgetView = ui->page_message;
    ui->contact_details_view->setCurrentWidget(ui->page_message);
    }

  getKeyhoteeWindow()->refreshDeleteContactOption();
  getKeyhoteeWindow()->refreshEditMenu();
  }

void ContactsTable::onDeleteContact()
  {
  //remove selected contacts from inbox model (and database)
  QSortFilterProxyModel* model = dynamic_cast<QSortFilterProxyModel*>(ui->contact_table->model());
  //model->setUpdatesEnabled(false);
  QItemSelectionModel*   selection_model = ui->contact_table->selectionModel();
  QModelIndexList        sortFilterIndexes = selection_model->selectedRows();
  if (sortFilterIndexes.count() == 0)
    return;
  if (QMessageBox::question(this, tr("Delete Contact"), tr("Are you sure you want to delete selected contact(s)?")) == QMessageBox::Button::No)
    return;
  QModelIndexList        indexes;
  foreach(QModelIndex sortFilterIndex, sortFilterIndexes)
    indexes.append(model->mapToSource(sortFilterIndex));
  qSort(indexes);
  auto sourceModel = model->sourceModel();
  auto app = bts::application::instance();
  auto profile = app->get_profile();

  for (int i = indexes.count() - 1; i > -1; --i)
  {
    auto contact_id = ((AddressBookModel*)sourceModel)->getContact(indexes.at(i)).wallet_index;
    if(profile->isIdentityPresent(((AddressBookModel*)sourceModel)->getContact(indexes.at(i)).dac_id_string))
    {
      profile->removeIdentity(((AddressBookModel*)sourceModel)->getContact(indexes.at(i)).dac_id_string);
    }
    sourceModel->removeRows(indexes.at(i).row(), 1);
    Q_EMIT contactDeleted(contact_id); //emit signal so that ContactGui is also deleted
  }
  //model->setUpdatesEnabled(true);
  //TODO Remove fullname/bitname for deleted contacts from QCompleter

  qSort(sortFilterIndexes);
  selectNextRow(sortFilterIndexes.takeLast().row(), indexes.count());
  }

bool ContactsTable::isShowDetailsHidden()
  {
  return ui->contact_details_view->isHidden();
  }

void ContactsTable::on_actionShow_details_toggled(bool checked)
  {
  if (ContactView * currentView = getCurrentView ())
    if (currentView->isAddingNewContact ())
      return;

  if (checked)
    ui->contact_details_view->show();
  else
    ui->contact_details_view->hide();
  }

void ContactsTable::addContactView(ContactView& view) const
  {
  ui->contact_details_view->addWidget(&view);
  }

void ContactsTable::showView(ContactView& view) const
  {
  if (view.isAddingNewContact())
    {
    ui->contact_details_view->show();
    showContactsTable (false);
    //save current view and restore it after canceled
    _currentWidgetView = ui->contact_details_view->currentWidget();    
    }
  else
    showContactsTable (true);

  ui->contact_details_view->setCurrentWidget(&view);
  }

bool ContactsTable::checkSaving() const
  {
  if (ContactView * currentView = getCurrentView ())
    {
    return currentView->CheckSaving();
    }
  return true;
  }

void ContactsTable::addNewContact(ContactView& view) const
  {
  addContactView(view);
  showView(view);
  view.addNewContact ();
  }

void ContactsTable::onCanceledNewContact()
  {
  showContactsTable (true);
  assert (_currentWidgetView);
  ui->contact_details_view->setCurrentWidget(_currentWidgetView);
  }

void ContactsTable::onSavedNewContact(int idxNewContact)
  {
  selectRow(idxNewContact);
  showContactsTable (true);
  }

void ContactsTable::selectRow(int contact_id)
{
  QModelIndex idx = _addressbook_model->findModelIndex(contact_id);
  QModelIndex mapped_index = _sorted_addressbook_model->mapFromSource(idx);

  ui->contact_table->selectRow(mapped_index.row());
}

ContactView* ContactsTable::getCurrentView() const
  {
  return qobject_cast<ContactView *>(ui->contact_details_view->currentWidget());
  }

void ContactsTable::selectChat()
  {
  if (ContactView * currentView = getCurrentView ())
    if (!currentView->isAddingNewContact ())
      currentView->onChat ();
  }

void ContactsTable::contactRemoved()
  {
  if (ContactView * currentView = getCurrentView ())
      currentView->onInfo ();
  }

void ContactsTable::showContactsTable (bool visible) const
  {
  //disable table, because hide() function emits signal onSelectionChanged
  ui->contact_table->setEnabled (visible);
  if (visible)
    ui->contact_table->show ();
  else
    ui->contact_table->hide ();
  }

bool ContactsTable::canContinue() const
  {
  return checkSaving();
  }

bool ContactsTable::isSelection () const
  {
  QItemSelectionModel* selection_model = ui->contact_table->selectionModel();
  QModelIndexList      indexes = selection_model->selectedRows();
  return (indexes.size() > 0);
  }

bool ContactsTable::hasFocusContacts() const
{
  return ui->contact_table->hasFocus();
}

void ContactsTable::selectNextRow(int idx, int deletedRowCount) const
{
  int count = _addressbook_model->rowCount();
  if (count == 0)
    return;
  int nextIdx = idx + 1 - deletedRowCount;
  if (nextIdx < count)
    ui->contact_table->selectRow(nextIdx);
  else
    ui->contact_table->selectRow(count - 1);
}

void ContactsTable::selectAll()
{
  ui->contact_table->selectAll();
  ui->contact_table->setFocus();
}

bool ContactsTable::EscapeIfEditMode() const
{
  if (ContactView * currentView = getCurrentView ())
  {
    if (currentView->isEditing())
    {
      currentView->onCancel();
      return true;
    }    
  }
  return false;
}

QWidget* ContactsTable::getContactsTableWidget () const
{
  return static_cast<QWidget*>(ui->contact_table);
}

void ContactsTable::copy()
{  
  QString strContact = QString();
  QItemSelectionModel* selection_model = ui->contact_table->selectionModel();
  QModelIndexList      indexes = selection_model->selectedRows();

  for (int i = 0; i < indexes.count(); i++)
  {
    QModelIndex mapped_index = _sorted_addressbook_model->mapToSource(indexes[i]);
    if (!strContact.isEmpty())
      strContact += "\n";
    strContact += _addressbook_model->getContact(mapped_index).get_display_name().c_str();
  }
  
  if (!strContact.isEmpty())
  {
    QClipboard *clip = QApplication::clipboard();
    clip->setText (strContact);
  }
}
