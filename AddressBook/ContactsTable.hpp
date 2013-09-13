#pragma once
#include <QWidget>
#include <memory>

namespace Ui { class ContactsTable; }

class AddressBookModel;
class QSortFilterProxyModel;

class ContactsTable  : public QWidget
{
  Q_OBJECT
  public:
    ContactsTable( QWidget* parent = nullptr );
    ~ContactsTable();

    void setAddressBook( AddressBookModel* abook_model );

  Q_SIGNALS:
    void openContact( int contact_id );

  private:
    std::unique_ptr<Ui::ContactsTable> ui;
    AddressBookModel*                  _addressbook_model;
    QSortFilterProxyModel*             _sorted_addressbook_model;
};
