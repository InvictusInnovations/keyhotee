#pragma once

#include "ch/authprocessor.hpp"

#include "keyhoteeidpubkeywidget.hpp"

#include <QDialog>

namespace Ui {
class RequestAuthorization;
}

class RequestAuthorization : public QDialog
{
  Q_OBJECT

public:
  explicit RequestAuthorization(QWidget *parent, IAuthProcessor& auth_processor, AddressBookModel* addressbook_model);
  ~RequestAuthorization();

  void setKeyhoteeID(const QString& name);
  void setPublicKey(const QString& name);

  void enableAddContact(bool active);

Q_SIGNALS:
  void authorizationStatus(int wallet_index);

private:
  void checkAddAsNewContact();
  void addAsNewContact();
  void genExtendedPubKey(bts::extended_public_key &extended_pub_key);
  void setAuthorizationStatus();
  /** QDialog reimplementation to delete identity observer.
      "done" method is called when dialog is canceled or accepted
  */
  virtual void done(int code) override;

private slots:
  void onExtendPubKey(bool checked);
  void onAddAsNewContact(bool checked);
  void onSend();
  void onStateWidget(KeyhoteeIDPubKeyWidget::CurrentState state);

private:
  Ui::RequestAuthorization *ui;
  AddressBookModel*         _addressbook_model;
  IAuthProcessor&           _auth_processor;
};
