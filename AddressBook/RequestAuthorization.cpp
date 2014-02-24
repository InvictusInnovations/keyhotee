#include "RequestAuthorization.hpp"
#include "ui_RequestAuthorization.h"

#include "public_key_address.hpp"

RequestAuthorization::RequestAuthorization(QWidget *parent) :
    QDialog(parent), ui(new Ui::RequestAuthorization)
{
  ui->setupUi(this);

  ui->keyhoteeidpubkey->setMode(KeyhoteeIDPubKeyWidget::ModeWidget::RequestAuthorization);
  ui->button_send->setEnabled(false);
  ui->keyhoteeidpubkey->showCopyToClipboard(false);

  connect(this, &QDialog::accepted, this, &RequestAuthorization::onSend);
  connect(ui->keyhoteeidpubkey, &KeyhoteeIDPubKeyWidget::currentState, this, &RequestAuthorization::onStateWidget);
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
  auto                                          app = bts::application::instance();
  auto                                          profile = app->get_profile();
  auto                                          idents = profile->identities();
  bts::bitchat::private_contact_request_message request_msg;
  if (idents.size() )
  {
    //request_msg.from_first_name = idents[0].first_name;
    //request_msg.from_last_name = idents[0].last_name;
    //request_msg.from_keyhotee_id = "";
    //request_msg.greeting_message = ui->message->toPlainText().toStdString();          // ************************************************
    //request_msg.from_channel = bts::network::channel_id(1);

    //fc::ecc::private_key my_priv_key = profile->get_keychain().get_identity_key(idents[0].dac_id_string);
    //app->send_contact_request(request_msg, ui->keyhoteeidpubkey->getPublicKey(), my_priv_key);     // **************** !!!!!!!!! no request_msg
  }
}

void RequestAuthorization::onStateWidget(KeyhoteeIDPubKeyWidget::CurrentState state)
{
  switch(state)
  {
    case KeyhoteeIDPubKeyWidget::CurrentState::InvalidData:
      ui->button_send->setEnabled(false);;
      ui->add_contact->setEnabled(false);
      break;
    case KeyhoteeIDPubKeyWidget::CurrentState::OkKeyhoteeID:
    case KeyhoteeIDPubKeyWidget::CurrentState::OkPubKey:
      ui->add_contact->setEnabled(true);
      ui->button_send->setEnabled(true);
      break;
    case KeyhoteeIDPubKeyWidget::CurrentState::IsStored:
      ui->add_contact->setEnabled(false);
      ui->button_send->setEnabled(true);
      break;
    default:
      assert(false);
  }
}
