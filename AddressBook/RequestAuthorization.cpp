#include "RequestAuthorization.hpp"
#include "ui_RequestAuthorization.h"

#include "AddressBookModel.hpp"
#include "public_key_address.hpp"

#include <QAction>
#include <QMenu>
#include <QMessageBox>

RequestAuthorization::RequestAuthorization(QWidget *parent) :
    QDialog(parent), ui(new Ui::RequestAuthorization)
{
  ui->setupUi(this);

  ui->keyhoteeidpubkey->setMode(KeyhoteeIDPubKeyWidget::ModeWidget::RequestAuthorization);
  ui->keyhoteeidpubkey->setEditable(true);
  ui->button_send->setEnabled(false);
  ui->keyhoteeidpubkey->showCopyToClipboard(false);

  fillSelectIdentities();

  connect(ui->extend_public_key , &QCheckBox::toggled, this, &RequestAuthorization::onExtendPubKey);
  connect(ui->add_contact , &QGroupBox::toggled, this, &RequestAuthorization::onAddAsNewContact);
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
  std::vector<bts::addressbook::wallet_identity> identities = bts::get_profile()->identities();

  if(identities.size() < 2)
  {
    ui->identity_select->addItem(tr(identities[0].get_display_name().c_str()));
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

void RequestAuthorization::checkAddAsNewContact()
{
  if(ui->extend_public_key->isChecked() &&
      ui->add_contact->isEnabled() && !ui->add_contact->isChecked())
  {
    QMessageBox::information(this, tr("Information"), tr("To send the Extended Public Key is necessary to add a new contact."));
    ui->add_contact->setChecked(true);
  }
}

void RequestAuthorization::addAsNewContact()
{
  if(ui->add_contact->isEnabled() && ui->add_contact->isChecked())
  {
    Contact new_conntact;
    new_conntact.first_name       = ui->first_name->text().toStdString();
    new_conntact.last_name        = ui->last_name->text().toStdString();
    new_conntact.dac_id_string    = ui->keyhoteeidpubkey->getKeyhoteeID().toStdString();
    new_conntact.public_key       = ui->keyhoteeidpubkey->getPublicKey();
    new_conntact.privacy_setting  = bts::addressbook::secret_contact;
    new_conntact.setIcon(QIcon(":/images/user.png"));

    _address_book->storeContact(new_conntact);
  }
}

void RequestAuthorization::genExtendedPubKey(bts::extended_public_key &extended_pub_key)
{
  if(ui->extend_public_key->isChecked())
  {
    auto profile = bts::get_profile();
    auto idents = profile->identities();
    int  identity = ui->identity_select->currentIndex();
    auto addressbook = profile->get_addressbook();
    bts::addressbook::wallet_contact contact;
    if(!Utils::matchContact(ui->keyhoteeidpubkey->getPublicKey(), &contact))
      return;

    extended_pub_key = profile->get_keychain().get_public_account(idents[identity].dac_id_string, contact.wallet_index);
  }
}

void RequestAuthorization::setAuthorizationStatus()
{
  auto addressbook = bts::get_profile()->get_addressbook();
  bts::addressbook::wallet_contact contact;
  if(!Utils::matchContact(ui->keyhoteeidpubkey->getPublicKey(), &contact))
    return;

  contact.authorization_status = bts::addressbook::authorization_status::sent_request;
  addressbook->store_contact(contact);
}

void RequestAuthorization::onExtendPubKey(bool checked)
{
  checkAddAsNewContact();
}

void RequestAuthorization::onAddAsNewContact(bool checked)
{
  checkAddAsNewContact();
}

void RequestAuthorization::onSend()
{
  addAsNewContact();

  auto                                          app = bts::application::instance();
  auto                                          profile = bts::get_profile();
  auto                                          idents = profile->identities();
  bts::bitchat::private_contact_request_message request_msg;
  if (idents.size() )
  {
    int identity = ui->identity_select->currentIndex();
    request_msg.from_first_name = idents[identity].first_name;
    request_msg.from_last_name = idents[identity].last_name;
    request_msg.from_keyhotee_id = idents[identity].dac_id_string;
    request_msg.greeting_message = ui->message->toPlainText().toStdString();
    request_msg.from_channel = bts::network::channel_id(1);
    
    uint16_t request_param = ui->check_box_chat->isChecked();
    request_param |= ui->check_box_mail->isChecked() << 1;
    request_param |= ui->extend_public_key->isChecked() << 8;
    request_msg.request_param = request_param;
    request_msg.status = bts::bitchat::authorization_status::request;
    request_msg.recipient = ui->keyhoteeidpubkey->getPublicKey();

    genExtendedPubKey(request_msg.extended_pub_key);

    fc::ecc::private_key my_priv_key = profile->get_keychain().get_identity_key(idents[identity].dac_id_string);
    app->send_contact_request(request_msg, ui->keyhoteeidpubkey->getPublicKey(), my_priv_key);

    setAuthorizationStatus();
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
      checkAddAsNewContact();
      break;
    case KeyhoteeIDPubKeyWidget::CurrentState::IsStored:
      ui->add_contact->setEnabled(false);
      ui->button_send->setEnabled(true);
      break;
    default:
      assert(false);
  }
}
