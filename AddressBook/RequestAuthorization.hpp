#pragma once
#include <QDialog>
#include "keyhoteeidpubkeywidget.hpp"

namespace Ui {
class RequestAuthorization;
}

class RequestAuthorization : public QDialog
{
  Q_OBJECT

public:
  explicit RequestAuthorization(QWidget *parent = 0);
  ~RequestAuthorization();

  void setAddressBook(AddressBookModel* address_book);

  void setKeyhoteeID(const QString& name);
  void setPublicKey(const QString& name);

  void enableAddContact(bool active);

private:
  void fillSelectIdentities();
  void checkAddAsNewContact();
  void addAsNewContact();
  void genExtendedPubKey(bts::extended_public_key &extended_pub_key);
  void setAuthorizationStatus();

private slots:
  void onExtendPubKey(bool checked);
  void onAddAsNewContact(bool checked);
  void onSend();
  void onStateWidget(KeyhoteeIDPubKeyWidget::CurrentState state);

private:
  Ui::RequestAuthorization *ui;
  AddressBookModel*         _address_book;
};
