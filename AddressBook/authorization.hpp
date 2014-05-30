#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include "ch/authprocessor.hpp"

#include "keyhoteeidpubkeywidget.hpp"

#include <QWidget>

namespace Ui {
class AuthorizationView;
}
class QToolBar;
class AuthorizationItem;
class AddressBookModel;

class Authorization : public QWidget
{
  Q_OBJECT

public:
  typedef bts::bitchat::private_contact_request_message TRequestMessage;
  typedef bts::bitchat::message_header                  THeaderStoredMsg;
  typedef bts::bitchat::authorization_status            TAuthorizationStatus;
  typedef bts::addressbook::wallet_contact              TWalletContact;
  typedef bts::addressbook::wallet_identity             TWalletIdentity;
  typedef bts::addressbook::authorization_status        TContAuthoStatus;

  Authorization(IAuthProcessor& auth_processor, AddressBookModel* _addressbook_model,
    const TRequestMessage& msg, const THeaderStoredMsg& header, QWidget *parent = 0);
  ~Authorization();

  void processResponse();

Q_SIGNALS:
  void authorizationStatus(int wallet_index);

protected:
  void acceptExtendedPubKey() const;
  void setAuthorizationStatus(TAuthorizationStatus status);

protected:
  IAuthProcessor&       _auth_processor;
  TRequestMessage       _reqmsg;
  THeaderStoredMsg      _header;
  AddressBookModel*     _addressbook_model;
};

class AuthorizationView : public Authorization
{
  Q_OBJECT

public:
  typedef fc::ecc::public_key                             TPublicKey;
  typedef bts::extended_public_key                        TExtendPubKey;

  explicit AuthorizationView(IAuthProcessor& auth_processor, AddressBookModel* _addressbook_model,
    const TRequestMessage& msg, const THeaderStoredMsg& header);
  virtual ~AuthorizationView();

  void setOwnerItem(AuthorizationItem* item);

Q_SIGNALS:
  void itemAcceptRequest (AuthorizationItem* item);
  void itemBlockRequest (AuthorizationItem* item);
  void itemDenyRequest (AuthorizationItem* item);

protected:
  virtual void showEvent(QShowEvent *) override;

public:
  void onAccept();
  void onDeny();
  void onBlock();

private slots:
  void onAddAsNewContact(bool checked);
  void onStateWidget(KeyhoteeIDPubKeyWidget::CurrentState state);

private:
  void addAsNewContact();
  void genExtendedPubKey(std::string identity_dac_id, TExtendPubKey &extended_pub_key);
  void sendReply(TAuthorizationStatus status);

private:
  Ui::AuthorizationView *ui;

  TPublicKey            _from_pub_key;
  QToolBar*             _toolbar;
  QAction*              _accept;
  QAction*              _deny;
  QAction*              _block;
  AuthorizationItem*    _owner_item;
};

#endif // AUTHORIZATION_H
