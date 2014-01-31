#include "RequestAuthorization.hpp"
#include "ui_RequestAuthorization.h"

#include "public_key_address.hpp"

RequestAuthorization::RequestAuthorization(QWidget *parent) :
    QDialog(parent), ui(new Ui::RequestAuthorization)
{
  ui->setupUi(this);
  ui->button_send->setEnabled(false);
  ui->keyhoteeidpubkey->showCopyToClipboard(false);

//  connect(ui->keyhotee_id, &QLineEdit::textChanged, this, &RequestAuthorization::onKeyhoteeIDChanged);
//  connect(ui->public_key, &QLineEdit::textChanged, this, &RequestAuthorization::onPublicKeyChanged);
  connect(this, &QDialog::accepted, this, &RequestAuthorization::onSend);
}

RequestAuthorization::~RequestAuthorization()
{
  delete ui;
}

void RequestAuthorization::setKeyhoteeID(const QString& keyhotee_id)
{
  ui->keyhoteeidpubkey->setKeyhoteeID(keyhotee_id);
}

void RequestAuthorization::setPublicKey(const QString& public_key_string)
{
  ui->keyhoteeidpubkey->setPublicKey(public_key_string);
}

void RequestAuthorization::enableAddContact(bool active)
{
  ui->add_contact->setEnabled(active);
}

void RequestAuthorization::onSend()
{
}
