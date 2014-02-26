#include "RequestAuthorization.hpp"
#include "ui_RequestAuthorization.h"

#include "public_key_address.hpp"
#include "AddressBookModel.hpp"

#include <QAction>
#include <QMenu>

RequestAuthorization::RequestAuthorization(QWidget *parent) :
    QDialog(parent), ui(new Ui::RequestAuthorization)
{
  ui->setupUi(this);

  ui->keyhoteeidpubkey->setMode(KeyhoteeIDPubKeyWidget::ModeWidget::RequestAuthorization);
  ui->keyhoteeidpubkey->setEditable(true);
  ui->button_send->setEnabled(false);
  ui->keyhoteeidpubkey->showCopyToClipboard(false);

  fillSelectIdentities();

  connect(this, &QDialog::accepted, this, &RequestAuthorization::onSend);
  connect(ui->keyhoteeidpubkey, &KeyhoteeIDPubKeyWidget::currentState, this, &RequestAuthorization::onStateWidget);
}

RequestAuthorization::~RequestAuthorization()
{
  delete ui;
}

void RequestAuthorization::setAddressBook(AddressBookModel* address_book)
{
  _address_book = address_book;
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

void RequestAuthorization::fillSelectIdentities()
{
  auto profile = bts::application::instance()->get_profile();
  std::vector<bts::addressbook::wallet_identity> identities = profile->identities();

  if(identities.size() < 2)
  {
    ui->identity_select->setVisible(false);
    ui->identity_select_label->setVisible(false);
    ui->line->setVisible(false);
    return;
  }

  for(const auto& identity : identities)
  {
    std::string entry = identity.get_display_name();
    auto ipk = identity.public_key;
    assert(ipk.valid());

    ui->identity_select->addItem(tr(entry.c_str()));
  }
}

void RequestAuthorization::onSend()
{
  addAsNewContact();

  auto                                          app = bts::application::instance();
  auto                                          profile = app->get_profile();
  auto                                          idents = profile->identities();
  bts::bitchat::private_contact_request_message request_msg;
  if (idents.size() )
  {
    int identity = ui->identity_select->currentIndex();
    request_msg.from_first_name = idents[identity].first_name;
    request_msg.from_last_name = idents[identity].last_name;
    request_msg.from_keyhotee_id = idents[identity].dac_id_string;
    request_msg.greeting_message = ui->message->toPlainText().toUtf8().constData();
    request_msg.from_channel = bts::network::channel_id(1);
    
    uint16_t request_param = ui->check_box_chat->isChecked();
    request_param |= ui->check_box_mail->isChecked() << 1;
    request_param |= ui->extend_public_key->isChecked() << 8;
    request_msg.request_param = request_param;

//    if(ui->extend_public_key->isChecked())
//    {
//      request_msg.extended_pub_key = profile->get_keychain().get_public_account(idents[identity].dac_id_string, wallet_index);
//    }

    fc::ecc::private_key my_priv_key = profile->get_keychain().get_identity_key(idents[0].dac_id_string);
    app->send_contact_request(request_msg, ui->keyhoteeidpubkey->getPublicKey(), my_priv_key);
  }
}

void RequestAuthorization::addAsNewContact()
{
  if(ui->add_contact->isEnabled() && ui->add_contact->isChecked())
  {
    Contact new_conntact;
    new_conntact.first_name       = ui->first_name->text().toUtf8().constData();
    new_conntact.last_name        = ui->last_name->text().toUtf8().constData();
    new_conntact.dac_id_string    = ui->keyhoteeidpubkey->getKeyhoteeID().toUtf8().constData();
    new_conntact.public_key       = ui->keyhoteeidpubkey->getPublicKey();
    new_conntact.privacy_setting  = bts::addressbook::secret_contact;
    new_conntact.setIcon(QIcon(":/images/user.png"));

    _address_book->storeContact(new_conntact);
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
