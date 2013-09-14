#pragma once
#include <QMainWindow>
#include <memory>
#include <unordered_map>
#include <bts/addressbook/addressbook.hpp>

namespace Ui { class KeyhoteeMainWindow; }
class QTreeWidgetItem;
class ContactView;
class AddressBookModel;
class ApplicationDelegate;

struct ContactWidgets
{
    QTreeWidgetItem* tree_item;
    ContactView*     view;
};

class KeyhoteeMainWindow  : public QMainWindow 
{
  public:
      KeyhoteeMainWindow();
      ~KeyhoteeMainWindow();

      void newMessage();
      void addContact();
      void showContacts();
      void onSidebarSelectionChanged();
      void selectContactItem( QTreeWidgetItem* item );
      void selectIdentityItem( QTreeWidgetItem* item );
      void openContact( int contact_id );
      void sideBarSplitterMoved( int pos, int index );
      ContactView* getContactView( int contact_id );
  private:
      friend class ApplicationDelegate;
      void addressBookDataChanged( const QModelIndex& top_left, const QModelIndex& bottom_right, const QVector<int>& roles );
      QTreeWidgetItem*                        _identities_root;
      QTreeWidgetItem*                        _mailboxes_root;
      QTreeWidgetItem*                        _contacts_root;
      QTreeWidgetItem*                        _inbox_root;
      QTreeWidgetItem*                        _drafts_root;
      QTreeWidgetItem*                        _sent_root;
      AddressBookModel*                       _addressbook_model;
      bts::addressbook::addressbook_ptr       _addressbook;
      std::unordered_map<int,ContactWidgets>  _contact_widgets;
      std::unique_ptr<Ui::KeyhoteeMainWindow> ui;
      std::unique_ptr<ApplicationDelegate>    _app_delegate;
};
