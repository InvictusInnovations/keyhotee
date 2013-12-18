#pragma once
#include <QWidget>
#include <memory>

namespace Ui { class ContactsTable; }

class AddressBookModel;
class QSortFilterProxyModel;
class ContactView;

class ContactsTable  : public QWidget
{
  Q_OBJECT
public:
  ContactsTable(QWidget* parent = nullptr);
  ~ContactsTable();

  void setAddressBook(AddressBookModel* addressbook_model);
  void openContact(const QModelIndex &current, const QModelIndex &previous);
  void searchEditChanged(QString search_string);
  void addContactView(ContactView& view) const;
  void showView(ContactView& view) const;
  void addNewContact(ContactView& view) const;
  bool isShowDetailsHidden();
  void onCanceledAddContact();
  bool CheckSaving(ContactView& newView) const;

  void selectRow(int index);

Q_SIGNALS:
  void contactOpened(int contact_id);
  void showPrevView();

private:
  std::unique_ptr<Ui::ContactsTable> ui;
  AddressBookModel*                  _addressbook_model;
  QSortFilterProxyModel*             _sorted_addressbook_model;
  QAction*                           _delete_contact;

public slots:
  void onDeleteContact();
  void on_actionShow_details_toggled(bool checked);
  void onCurrentViewChanged(int index);
};
