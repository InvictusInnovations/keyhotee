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
  Ui::RequestAuthorization *ui;
  AddressBookModel*         _address_book;

  void fillSelectIdentities();

  void onSend();
  void addAsNewContact();
  void onStateWidget(KeyhoteeIDPubKeyWidget::CurrentState state);
};
