#pragma once
#include <QWidget>
#include <memory>

#include "ch/ModificationsChecker.hpp"

#include <bts/addressbook/contact.hpp>
#include <QList>

namespace Ui { class ContactsTable; }

class AddressBookModel;
class QSortFilterProxyModel;
class QItemSelection;
class ContactView;
class Contact;

class ContactsTable  : public QWidget,
                       protected IModificationsChecker
{
  Q_OBJECT
public:
  ContactsTable(QWidget* parent = nullptr);
  virtual ~ContactsTable();

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
  bool isSelection () const;
  bool hasFocusContacts() const;
  void selectAll ();
  bool EscapeIfEditMode() const;
  QWidget* getContactsTableWidget () const;
  void copy();
  void getSelectedContacts (QList<const Contact*>& contacts);
  void setShowBlocked(bool show);
  void updateOptions();

private:
  /// \see IModificationsChecker interface description.
  virtual bool canContinue() const override;

  ContactView* getCurrentView() const;
  void showContactsTable (bool visible) const;
  void selectNextRow(int idx, int deletedRowCount) const;  
  /** Update header title.
      Display "Contact list" or "Blocked contacts list"
  */
  void updateHeader();
  bool deleteIdentity(bts::addressbook::wallet_identity& contact);
  
Q_SIGNALS:
  void contactOpened(int contact_id);
  void contactDeleted(int contact_id);

private:
  Ui::ContactsTable*                 ui;
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
