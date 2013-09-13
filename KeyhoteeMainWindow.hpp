#pragma once
#include <QMainWindow>
#include <memory>
#include <unordered_map>

namespace Ui { class KeyhoteeMainWindow; }
class QTreeWidgetItem;
class ContactView;
class AddressBookModel;

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

  private:
      QTreeWidgetItem*                        _identities_root;
      QTreeWidgetItem*                        _mailboxes_root;
      QTreeWidgetItem*                        _contacts_root;
      QTreeWidgetItem*                        _inbox_root;
      QTreeWidgetItem*                        _drafts_root;
      QTreeWidgetItem*                        _sent_root;
      AddressBookModel*                       _addressbook_model;
      std::unordered_map<int,ContactWidgets>  _contact_widgets;
      std::unique_ptr<Ui::KeyhoteeMainWindow> ui;
};
