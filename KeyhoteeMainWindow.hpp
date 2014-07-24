#pragma once
#include <memory>
#include <unordered_map>
#include <bts/addressbook/addressbook.hpp>
#include <bts/application.hpp>
#include "AddressBook/ContactGui.hpp"

#include "ATopLevelWindowsContainer.hpp"
#include "ConnectionProcessor.hpp"

#include "AddressBook/AuthorizationItem.hpp"

#include "ch/GuiUpdateSink.hpp"
#include "ch/ModificationsChecker.hpp"
#include "Identity/IdentitiesUpdate.hpp"

#include "qtreusable/selfsizingmainwindow.h"

#include "fc/io/buffered_iostream.hpp"

#include <QList>
#include <QTreeWidget>

namespace Ui { class KeyhoteeMainWindow; }

class AddressBookModel;
class AuthorizationView;
class Contact;
class ContactGui;
class ContactView;
class ContactsTable;
class InboxView;
class IManageWallet;
class Mailbox;
class MailboxModel;
class MailboxModelRoot;
class MenuEditControl;
class KeyhoteeMainWindow;
class TKeyhoteeApplication;
class WalletsGui;

class QAction;
class QCompleter;
class QLineEdit;
class QTreeWidgetItem;

class KeyhoteeMainWindow  : public ATopLevelWindowsContainer,
                            protected IModificationsChecker,
                            protected IGuiUpdateSink,
                            protected IIdentitiesUpdate
{
  Q_OBJECT
public:

  void newMailMessage();
  void newMailMessageTo(const Contact& contact);
  void addToContacts(const bts::addressbook::wallet_contact& wallet_contact);
  /** Allows to add new contact(s) to address book
      \param silent - false: show window "Add new contact" and fill contact fields,
                      true:  add directly all contacts to the address book 
                             without showing preview window
  */
  void addToContacts(bool silent, std::list<Contact> &contacts);
  void selectContactItem(QTreeWidgetItem* item);
  void selectIdentityItem(QTreeWidgetItem* item);
  void sideBarSplitterMoved(int pos, int index);

  void openContactGui(int contact_id);
  ContactGui* getContactGui(int contact_id);
  ContactGui* createContactGuiIfNecessary(int contact_id);
  bool isSelectedContactGui(ContactGui* contactGui);  

  void displayDiagnosticLog();
  void setEnabledAttachmentSaveOption(bool enable);
  void setEnabledDeleteOption( bool enable );
  void setEnabledContactOption( bool enable );
  void refreshMenuOptions();
  /// Called from Mailbox::CheckSendMailButtons()
  void onEnableMailButtons(bool enable);
  ContactsTable* getContactsPage();
  void shareContact(QList<const Contact*>& contacts);

  AddressBookModel* getAddressBookModel() const { return _addressbook_model; }
  TConnectionProcessor* getConnectionProcessor() { return &_connectionProcessor; }

  void onUpdateAuthoStatus(int contact_id);

signals:
  void checkSendMailSignal();
protected:
  virtual void closeEvent(QCloseEvent *) override;
  virtual void keyPressEvent(QKeyEvent *) override;

private:
/// IGuiUpdateSink interface description:
  /// \see IGuiUpdateSink interface description.
  virtual void OnReceivedChatMessage(const TContact& sender, const TChatMessage& msg,
    const TTime& timeSent) override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnReceivedAuthorizationMessage(const TAuthorizationMessage& msg,
    const TStoredMailMessage& header) override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnReceivedMailMessage(const TStoredMailMessage& msg, const bool spam) override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnReceivedUnsupportedMessage(const TDecryptedMessage& msg) override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnMessageSaving() override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnMessageSaved(const TStoredMailMessage& msg,
    const TStoredMailMessage* overwrittenOne) override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnMessageGroupPending(unsigned int count) override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnMessagePending(const TStoredMailMessage& msg,
    const TStoredMailMessage* savedDraftMsg) override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnMessageGroupPendingEnd() override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnMessageSendingStart() override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnMessageSent(const TStoredMailMessage& pendingMsg,
    const TStoredMailMessage& sentMsg, const TDigest& digest) override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnMessageSendingEnd() override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnMissingSenderIdentity(const TRecipientPublicKey& senderId,
    const TPhysicalMailMessage& msg) override;

  /// Only TKeyhoteeApplication can build main window object.
  friend class TKeyhoteeApplication;
  KeyhoteeMainWindow(const TKeyhoteeApplication& mainApp);
  virtual ~KeyhoteeMainWindow();
  /// Helper method to simplify onSidebarSelectionChanged code.
  void activateMailboxPage(Mailbox* mailBox);

  /// \see IModificationsChecker interface description.
  virtual bool canContinue() const override;


  void onWalletsNotification(const QString& str);

protected:
  /// \see IIdentitiesUpdate interface description.
  virtual void onIdentitiesChanged(const TIdentities& identities) override {}
  virtual bool onIdentityDelIntent(const TIdentity&  identity) override;
  virtual bool onIdentityDelete(const TIdentity&  identity) override;

private slots:
  // ---------- MenuBar
  // File
  void onOptions();
  void onExit();
  // Edit
  void onCopy();
  void onCut();
  void onPaste();
  void onSelectAll();
  // Identity
  void onNewIdentity();
  void enableNewMessageIcon();
  void onEnableMining(bool enabled);
  // Mail
  void onSaveAttachement();
  // Contact
  void onSetIcon();
  void onRequestAuthorization();
  void onShowBlockedContacts();
  void onBlockContact();
  void onUnblockContact();
  void onShareContact();
  // Help
  void onDiagnostic();
  void onAbout();

  void onCanceledNewContact();
  void onSavedNewContact(int idxNewContact);
  void onItemContactRemoved (QTreeWidgetItem&);
  void onRemoveContact ();
  void onClipboardChanged();
  void onFocusChanged(QWidget *old, QWidget *now);
  void onDeleteAuthorizationItem();
  void onItemContextAcceptRequest(QTreeWidgetItem *item);
  void onItemContextDenyRequest(QTreeWidgetItem *item);
  void onItemContextBlockRequest(QTreeWidgetItem *item);
  void onItemAcceptRequest(AuthorizationItem *item);
  void onItemDenyRequest(AuthorizationItem *item);
  void onItemBlockRequest(AuthorizationItem *item);

  void onSidebarSelectionChanged();
  void onSidebarDoubleClicked();

  void onUpdateOptions(bool lang_changed);

private:
  bool isIdentityPresent();
  void addressBookDataChanged(const QModelIndex& top_left, const QModelIndex& bottom_right,
                              const QVector<int>& roles);
  void searchEditChanged(QString search_string);

  void createContactGui(int contact_id);
  void showContactGui(ContactGui& contact_gui);
  void deleteContactGui(int contact_id);
  void setupStatusBar();
  void notSupported();
  void enableMenu(bool enable);
  /** Allows to stop mail transmission if any before quit.
      Returns true if quit operation can be continued.
  */
  bool stopMailTransmission();
  bool checkSaving() const;
  void showContacts();
  void createAuthorizationItem(const TAuthorizationMessage& msg, const TStoredMailMessage& header);
  QTreeWidgetItem* findExistSenderItem(AuthorizationItem::TPublicKey from_key, bool &to_root);
  void showAuthorizationItem(AuthorizationItem *item);
  void deleteAuthorizationItem(AuthorizationItem *item);
  void addContact();
  void processResponse(const TAuthorizationMessage& msg, const TStoredMailMessage& header);
  void loadStoredRequests(bts::bitchat::message_db_ptr request_db);
  void enableBlockedContact(bool enable);

  /** Load wallets data from WalletsGui.xml file and initialize Wallets tree.
   * User can edit and add new wallets to WalletsGui.xml file.
   * WalletsGui.xml file exists in the ../Keyhotee.exe directory.
   * If Wallets.xml file doesn't exist Application copy it from resource ":Wallets/DefaultWallets.xml"
   */
  void setupWallets();

  void startBitsharesClient();
  int writeToStream(fc::buffered_ostream_ptr stream, std::string str);

  /// Class attributes:
private:
  typedef std::map<QTreeWidgetItem*, IManageWallet*> TTreeItem2ManageWallet;

  QTreeWidgetItem*                        _identities_root;
  QTreeWidgetItem*                        _mailboxes_root;
  QTreeWidgetItem*                        _wallets_root;
  QTreeWidgetItem*                        _contacts_root;
  QTreeWidgetItem*                        _requests_root;
  QTreeWidgetItem*                        _inbox_root;
  QTreeWidgetItem*                        _drafts_root;
  QTreeWidgetItem*                        _out_box_root;
  QTreeWidgetItem*                        _sent_root;
  QTreeWidgetItem*                        _spam_root;
  QList<QTreeWidgetItem*>                 _walletItems;
  /// map tree item widget to wallet web site
  TTreeItem2ManageWallet                  _tree_item_2_wallet;

  MailboxModel*                           _inbox_model;
  MailboxModel*                           _draft_model;
  MailboxModel*                           _pending_model;
  MailboxModel*                           _sent_model;
  MailboxModel*                           _spam_model;
  MailboxModelRoot*                       _mail_model_root;

  AddressBookModel*                       _addressbook_model;
  std::unordered_map<int, ContactGui>     _contact_guis;

  QLineEdit*                              _search_edit;
  Ui::KeyhoteeMainWindow*                 ui;
  TConnectionProcessor                    _connectionProcessor;
  Mailbox*                                _currentMailbox;
  MenuEditControl*                        menuEdit;
  QList <Mailbox*>                        _mailboxesList;
  /// Set to true when close event processing is in progress (it wasn't yet accepted nor ignored)
  bool                                    _isClosing;
  QList <QAction*>                        _actionsLang;
  std::unique_ptr<WalletsGui>             _walletsGui;
  bool                                    _is_curr_contact_blocked;
  bool                                    _is_curr_contact_own;
  bool                                    _is_filter_blocked_on;
  bool                                    _is_show_blocked_contacts;
  TIdentity                               _identity_replace;
  bool                                    _bitshares_client_on_startup;
  QString                                 _rpc_username;
  QString                                 _rpc_password;
}; //KeyhoteeMainWindow

KeyhoteeMainWindow* getKeyhoteeWindow();
