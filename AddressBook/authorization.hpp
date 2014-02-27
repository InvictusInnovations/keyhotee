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
  typedef bts::bitchat::decrypted_message                 TDecryptedMessage;
  typedef bts::bitchat::private_contact_request_message   TRequestMessage;
  typedef fc::ecc::public_key                             TPublicKey;
  typedef bts::extended_public_key                        TExtendPubKey;
  typedef bts::bitchat::authorization_status              TAuthoriztionStatus;

  explicit Authorization(QWidget *parent = 0);
  virtual ~Authorization();

  void setAddressBook(AddressBookModel* addressbook);

  void setMsg(const TDecryptedMessage& msg);
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
  void genExtendedPubKey(std::string dac_id, TExtendPubKey &extended_pub_key);
  void sendReply(TAuthoriztionStatus status);

private:
  Ui::Authorization *ui;

  //TDecryptedMessage     _msg;
  TRequestMessage       _reqmsg;
  TPublicKey            _from_pub_key;
//  TExtendPubKey         _extend_pub_key;
//  TPublicKey            _my_pub_key;
  QToolBar*             _toolbar;
  QAction*              _accept;
  QAction*              _deny;
  QAction*              _block;
  AuthorizationItem*    _owner_item;
  AddressBookModel*     _address_book;
};

#endif // AUTHORIZATION_H
