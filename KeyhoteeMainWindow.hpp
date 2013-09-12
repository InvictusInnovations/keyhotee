#pragma once
#include <QMainWindow>
#include <memory>

namespace Ui { class KeyhoteeMainWindow; }
class QTreeWidgetItem;
class ContactView;
class AddressBookModel;

class KeyhoteeMainWindow  : public QMainWindow 
{
  public:
      KeyhoteeMainWindow();
      ~KeyhoteeMainWindow();

      void addContact();
      void showContacts();
      void onSidebarSelectionChanged();
      void selectContactItem( QTreeWidgetItem* item );
      void selectIdentityItem( QTreeWidgetItem* item );

  private:
      QTreeWidgetItem*  _identities_root;
      QTreeWidgetItem*  _mailboxes_root;
      QTreeWidgetItem*  _contacts_root;
      QTreeWidgetItem*  _inbox_root;
      QTreeWidgetItem*  _drafts_root;
      QTreeWidgetItem*  _sent_root;
      AddressBookModel* _addressbook_model;
      std::unique_ptr<Ui::KeyhoteeMainWindow> ui;
};
