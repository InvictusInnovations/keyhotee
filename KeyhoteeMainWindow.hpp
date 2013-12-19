#pragma once
#include <memory>
#include <unordered_map>
#include <bts/addressbook/addressbook.hpp>
#include <bts/application.hpp>

#include "qtreusable/selfsizingmainwindow.h"

#include "dataaccessimpl.h"
#include "mailprocessorimpl.hpp"

namespace Ui { class KeyhoteeMainWindow; }

class QTreeWidgetItem;
class QLineEdit;
class QCompleter;

class AddressBookModel;
class Contact;
class ContactView;
class InboxView;
class MailboxModel;
class KeyhoteeMainWindow;

/**
 *  GUI widgets and GUI state for a contact.
 *  Not all contacts have this, only contacts "active" in GUI.
 */
class ContactGui
{
  friend KeyhoteeMainWindow;

private:
  unsigned int     _unread_msg_count;
  QTreeWidgetItem* _tree_item;
  ContactView*     _view;

public:
  ContactGui() {}
  ContactGui(QTreeWidgetItem* tree_item, ContactView* view)
    : _tree_item(tree_item), _view(view), _unread_msg_count(0) {}

  void updateTreeItemDisplay();
  void setUnreadMsgCount(unsigned int count);
  bool isChatVisible();
  void receiveChatMessage(const QString& from, const QString& msg, const QDateTime& dateTime);
private:
};

class KeyhoteeMainWindow  : public SelfSizingMainWindow,
  protected bts::application_delegate
{
public:
  KeyhoteeMainWindow();
  virtual ~KeyhoteeMainWindow();

  void newMailMessage();
  void newMailMessageTo(const Contact& contact);
  void addContact();
  void showContacts();
  void onSidebarSelectionChanged();
  void selectContactItem(QTreeWidgetItem* item);
  void selectIdentityItem(QTreeWidgetItem* item);
  void sideBarSplitterMoved(int pos, int index);

  void openContactGui(int contact_id);
  ContactGui* getContactGui(int contact_id);
  ContactGui* createContactGuiIfNecessary(int contact_id);
  bool isSelectedContactGui(ContactGui* contactGui);


  void openDraft(int draft_id);
  void openMail(int message_id);
  void openSent(int message_id);

protected:
  /// application_delegate interface implementation
  virtual void received_text(const bts::bitchat::decrypted_message& msg);
  virtual void received_email(const bts::bitchat::decrypted_message& msg);


private slots:
  // ---------- MenuBar
  // File
  void on_actionExit_triggered();
  // Edit
  void on_actionCopy_triggered();
  void on_actionCut_triggered();
  void on_actionPaste_triggered();
  void on_actionSelectAll_triggered();
  void on_actionDelete_triggered();
  // Identity
  void on_actionNew_identity_triggered();
  void enableMining_toggled(bool enabled);
  // Mail
  void on_actionReply_triggered();
  void on_actionReply_all_triggered();
  void on_actionForward_triggered();
  void on_actionSave_attachement_triggered();
  // Contact
  void on_actionset_Icon_triggered();
  // Help
  void on_actionDiagnostic_triggered();
  void on_actionAbout_triggered();

  void onShowPrevView();

private:
  void addressBookDataChanged(const QModelIndex& top_left, const QModelIndex& bottom_right,
                              const QVector<int>& roles);
  void searchEditChanged(QString search_string);

  void createContactGui(int contact_id);
  void showContactGui(ContactGui& contact_gui);
  void deleteContactGui(int contact_id);
  void setupStatusBar();
  void notSupported();

  /// Class attributes:

  //QCompleter*                             _contact_completer;
  QTreeWidgetItem*                        _identities_root;
  QTreeWidgetItem*                        _mailboxes_root;
  QTreeWidgetItem*                        _wallets_root;
  QTreeWidgetItem*                        _contacts_root;
  QTreeWidgetItem*                        _inbox_root;
  QTreeWidgetItem*                        _drafts_root;
  QTreeWidgetItem*                        _sent_root;
  QTreeWidgetItem*                        _bitcoin_root;
  QTreeWidgetItem*                        _bitshares_root;
  QTreeWidgetItem*                        _litecoin_root;

  MailboxModel*                           _inbox_model;
  MailboxModel*                           _draft_model;
  MailboxModel*                           _pending_model;
  MailboxModel*                           _sent_model;

  AddressBookModel*                       _addressbook_model;
  bts::addressbook::addressbook_ptr       _addressbook;
  std::unordered_map<int, ContactGui>     _contact_guis;

  QLineEdit*                              _search_edit;
  std::unique_ptr<Ui::KeyhoteeMainWindow> ui;
  TConnectionStatusDS                     ConnectionStatusDS;
  TMailProcessor                          MailProcessor;
}; //KeyhoteeMainWindow

KeyhoteeMainWindow* GetKeyhoteeWindow();