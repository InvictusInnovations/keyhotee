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
class QCompleter;
class InboxView;
class InboxModel;
class KeyhoteeMainWindow;

/**
 *  GUI widgets and GUI state for a contact.
 *  Not all contacts have this, only contacts "active" in GUI.
 */
class ContactGui
{

friend KeyhoteeMainWindow;

private:
    unsigned int     _unreadMsgCount;
    QTreeWidgetItem* _tree_item;
    ContactView*     _view;

public:
         ContactGui() {}
         ContactGui(QTreeWidgetItem* tree_item, ContactView* view)
         : _tree_item(tree_item), _view(view), _unreadMsgCount(0) {}

    void updateTreeItemDisplay();
    void setUnreadMsgCount(unsigned int count);
    bool isChatVisible();
    void receiveChatMessage( const QString& from, const QString& msg, const QDateTime& dateTime);
private:
};

class KeyhoteeMainWindow  : public QMainWindow 
{
  public:
      KeyhoteeMainWindow();
      ~KeyhoteeMainWindow();

      void         newMessage();
      void         addContact();
      void         showContacts();
      void         onSidebarSelectionChanged();
      void         selectContactItem( QTreeWidgetItem* item );
      void         selectIdentityItem( QTreeWidgetItem* item );
      void         sideBarSplitterMoved( int pos, int index );

      void         openContactGui( int contact_id );
      ContactGui*  getContactGui( int contact_id );
      ContactGui*  createContactGuiIfNecessary( int contact_id );
      bool         isSelectedContactGui(ContactGui* contactGui);


      void         openDraft( int draft_id  );
      void         openMail( int message_id );
      void         openSent( int message_id );

     
  private:
      friend class ApplicationDelegate;
      void addressBookDataChanged( const QModelIndex& top_left, const QModelIndex& bottom_right, const QVector<int>& roles );

      void    createContactGui( int contact_id );
      void    showContactGui( ContactGui& contact_gui );

      QCompleter*                             _contact_completer;
      QTreeWidgetItem*                        _identities_root;
      QTreeWidgetItem*                        _mailboxes_root;
      QTreeWidgetItem*                        _wallets_root;
      QTreeWidgetItem*                        _contacts_root;
      QTreeWidgetItem*                        _inbox_root;
      QTreeWidgetItem*                        _drafts_root;
      QTreeWidgetItem*                        _sent_root;

      InboxModel*                             _inbox;
      AddressBookModel*                       _addressbook_model;
      bts::addressbook::addressbook_ptr       _addressbook;
      std::unordered_map<int,ContactGui>      _contact_guis;
      std::unique_ptr<Ui::KeyhoteeMainWindow> ui;
      std::unique_ptr<ApplicationDelegate>    _app_delegate;
};

KeyhoteeMainWindow* GetKeyhoteeWindow();