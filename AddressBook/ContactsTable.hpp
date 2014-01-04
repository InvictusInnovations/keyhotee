#pragma once
#include <QWidget>
#include <memory>

#include "ch/ModificationsChecker.hpp"

namespace Ui { class ContactsTable; }

class AddressBookModel;
class QSortFilterProxyModel;
class QItemSelection;
class ContactView;

class ContactsTable  : public QWidget,
                       public IModificationsChecker
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
  void selectChat();
  void contactRemoved ();
  virtual bool canContinue() const;
  bool isSelection () const;
  bool hasFocusContacts() const;

private:
  ContactView* getCurrentView() const;
  void showContactsTable (bool visible) const;

Q_SIGNALS:
  void contactOpened(int contact_id);
  void contactDeleted(int contact_id);

private:
  std::unique_ptr<Ui::ContactsTable> ui;
  AddressBookModel*                  _addressbook_model;
  QSortFilterProxyModel*             _sorted_addressbook_model;
  QAction*                           _delete_contact;
  mutable QWidget*                   _currentWidgetView;

public slots:
  void onDeleteContact();
  void on_actionShow_details_toggled(bool checked);
  void onSavedNewContact(int idxNewContact);
  void onCanceledNewContact();
};
