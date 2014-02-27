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
  explicit Authorization(QWidget *parent = 0);
  ~Authorization();

  void setAddressBook(AddressBookModel* addressbook);

  void setMsg(const bts::bitchat::decrypted_message& msg);
  void setOwnerItem(AuthorizationItem* item);

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

  typedef bts::bitchat::decrypted_message               TDecryptedMessage;
  typedef bts::bitchat::private_contact_request_message TRequestMessage;
  typedef fc::ecc::public_key                           TPublicKey;
  typedef bts::extended_public_key                      TExtendPubKey;

  Ui::Authorization *ui;

  TDecryptedMessage     _msg;
  TExtendPubKey         _extend_pub_key;
  QToolBar*             _toolbar;
  QAction*              _accept;
  QAction*              _deny;
  QAction*              _block;
  AuthorizationItem*    _owner_item;
  AddressBookModel*     _address_book;
};

#endif // AUTHORIZATION_H
