#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include <QWidget>
#include "keyhoteeidpubkeywidget.hpp"

namespace Ui {
class Authorization;
}
class QToolBar;
class AuthorizationItem;
class AddressBookModel;

class Authorization : public QWidget
{
  Q_OBJECT

public:
  typedef bts::bitchat::private_contact_request_message   TRequestMessage;
  typedef fc::ecc::public_key                             TPublicKey;
  typedef bts::extended_public_key                        TExtendPubKey;
  typedef bts::bitchat::authorization_status              TAuthorizationStatus;
  typedef bts::addressbook::authorization_status          TContAuthoStatus;
  typedef bts::addressbook::wallet_contact                TWalletContact;

  explicit Authorization(QWidget *parent = 0);
  virtual ~Authorization();

  void setAddressBook(AddressBookModel* addressbook);

  void setMsg(const TPublicKey& sender, const TRequestMessage& msg);
  void setOwnerItem(AuthorizationItem* item);
  void processResponse();

Q_SIGNALS:
  void itemAcceptRequest (AuthorizationItem* item);
  void itemBlockRequest (AuthorizationItem* item);
  void itemDenyRequest (AuthorizationItem* item);

public:
  void onAccept();
  void onDeny();
  void onBlock();

private slots:
  void onAddAsNewContact(bool checked);
  void onStateWidget(KeyhoteeIDPubKeyWidget::CurrentState state);

private:
  void addAsNewContact();
  void acceptExtendedPubKey();
  void genExtendedPubKey(std::string identity_dac_id, TExtendPubKey &extended_pub_key);
  void sendReply(TAuthorizationStatus status);
  void setAuthorizationStatus(TAuthorizationStatus status);

private:
  Ui::Authorization *ui;

  TRequestMessage       _reqmsg;
  TPublicKey            _from_pub_key;
  QToolBar*             _toolbar;
  QAction*              _accept;
  QAction*              _deny;
  QAction*              _block;
  AuthorizationItem*    _owner_item;
  AddressBookModel*     _address_book;
};

#endif // AUTHORIZATION_H
