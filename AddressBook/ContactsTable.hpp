#pragma once
#include <QWidget>
#include <memory>

namespace Ui { class ContactsTable; }

class AddressBookModel;
class QSortFilterProxyModel;
class QItemSelection;
class ContactView;

class ContactsTable  : public QWidget
{
  Q_OBJECT
public:
  ContactsTable(QWidget* parent = nullptr);
  ~ContactsTable();

  void setAddressBook(AddressBookModel* addressbook_model);
  void onSelectionChanged (const QItemSelection &selected, const QItemSelection &deselected);
  void searchEditChanged(QString search_string);
  void addContactView(ContactView& view) const;
  void showView(ContactView& view) const;
  void addNewContact(ContactView& view) const;
  bool isShowDetailsHidden();
  bool checkSaving() const;
  void selectRow(int index);

private:
  ContactView* getCurrentView() const;

Q_SIGNALS:
  void contactOpened(int contact_id);
  void contactDeleted(int contact_id);

private:
  std::unique_ptr<Ui::ContactsTable> ui;
  AddressBookModel*                  _addressbook_model;
  QSortFilterProxyModel*             _sorted_addressbook_model;
  QAction*                           _delete_contact;

public slots:
  void onDeleteContact();
  void on_actionShow_details_toggled(bool checked);
  void onCurrentViewChanged(int index);
  void onSavedNewContact();
  void onCanceledNewContact();
};
