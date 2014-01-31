#include "RequestAuthorization.hpp"
#include "ui_RequestAuthorization.h"

#include "public_key_address.hpp"

RequestAuthorization::RequestAuthorization(QWidget *parent) :
    QDialog(parent), ui(new Ui::RequestAuthorization)
{
  ui->setupUi(this);
  ui->button_send->setEnabled(false);

//  connect(ui->keyhotee_id, &QLineEdit::textChanged, this, &RequestAuthorization::onKeyhoteeIDChanged);
//  connect(ui->public_key, &QLineEdit::textChanged, this, &RequestAuthorization::onPublicKeyChanged);
  connect(this, &QDialog::accepted, this, &RequestAuthorization::onSend);
}

RequestAuthorization::~RequestAuthorization()
{
  delete ui;
}

void RequestAuthorization::onKeyhoteeIDChanged(const QString& keyhotee_id)
{
  //if(keyhotee_id.isEmpty())
  //  ui->public_key->setEnabled(true);
  //else
  //  ui->public_key->setEnabled(false);

  //ui->button_send->setEnabled(true);
}

void RequestAuthorization::onPublicKeyChanged(const QString& public_key_string)
{
  //if(public_key_string.isEmpty())
  //{
  //  ui->keyhotee_id->setEnabled(true);
  //  ui->public_key->setStyleSheet("");
  //  return;
  //}
  //else
  //  ui->keyhotee_id->setEnabled(false);

  //bool publicKeySemanticallyValid = false;
  //bool public_key_is_valid = public_key_address::is_valid(public_key_string.toStdString(), 
  //                                                        &publicKeySemanticallyValid);

  //if(publicKeySemanticallyValid)
  //{
  //  ui->public_key->setStyleSheet("border: 1px solid green");
  //  ui->button_send->setEnabled(true);
  //}
  //else
  //{
  //  ui->public_key->setStyleSheet("border: 1px solid red");
  //  ui->button_send->setEnabled(false);
  //}
}

void RequestAuthorization::onSend()
{
}
