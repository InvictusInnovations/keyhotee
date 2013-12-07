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

    void setAddressBook( AddressBookModel* addressbook_model );
    void openContact( const QModelIndex& index );
    void searchEditChanged(QString search_string);

    bool isShowDetailsHidden();
  Q_SIGNALS:
    void contactOpened( int contact_id );

  private:
    std::unique_ptr<Ui::ContactsTable> ui;
    AddressBookModel*                  _addressbook_model;
    QSortFilterProxyModel*             _sorted_addressbook_model;
    QAction*                           _delete_contact;

  public slots:
    void onDeleteContact();
    void on_actionShow_details_toggled(bool checked);
};
