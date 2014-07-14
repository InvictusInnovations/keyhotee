#include "RequestAuthorization.hpp"
#include "ui_RequestAuthorization.h"

#include "AddressBookModel.hpp"
#include "public_key_address.hpp"

#include "Identity/IdentityObservable.hpp"
#include "Identity/IdentitySelection.hpp"

#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include <string>

RequestAuthorization::RequestAuthorization(QWidget *parent, IAuthProcessor& auth_processor, AddressBookModel* addressbook_model) :
  QDialog(parent), ui(new Ui::RequestAuthorization),
  _auth_processor(auth_processor),
  _addressbook_model(addressbook_model)
{
  ui->setupUi(this);

  ui->keyhoteeidpubkey->setMode(KeyhoteeIDPubKeyWidget::ModeWidget::RequestAuthorization);
  ui->keyhoteeidpubkey->setEditable(true);
  ui->button_send->setEnabled(false);
  ui->keyhoteeidpubkey->hideCopyKeysToClipboard();
  ui->keyhoteeidpubkey->setFocus(Qt::ActiveWindowFocusReason);
  ui->keyhoteeidpubkey->setAddressBook(_addressbook_model);
    
  ui->widget_Identity->addWidgetRelated(ui->line);
  /// add identity observer
  IdentityObservable::getInstance().addObserver( ui->widget_Identity );

  connect(ui->extend_public_key , &QCheckBox::toggled, this, &RequestAuthorization::onExtendPubKey);
  connect(ui->add_contact , &QGroupBox::toggled, this, &RequestAuthorization::onAddAsNewContact);
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

    _addressbook_model->storeContact(new_conntact);
  }
}

void RequestAuthorization::genExtendedPubKey(bts::extended_public_key &extended_pub_key)
{
  if(ui->extend_public_key->isChecked())
  {    
    bts::addressbook::wallet_contact contact;
    if(!Utils::matchContact(ui->keyhoteeidpubkey->getPublicKey(), &contact))
      return;

    auto identity = ui->widget_Identity->currentIdentity();
    if (identity != nullptr)
    {
      auto profile = bts::get_profile();
      extended_pub_key = profile->get_keychain().get_public_account(identity->dac_id_string, contact.wallet_index);
    }
  }
}

void RequestAuthorization::setAuthorizationStatus()
{
  bts::addressbook::wallet_contact contact_tmp;
  if(!Utils::matchContact(ui->keyhoteeidpubkey->getPublicKey(), &contact_tmp))
    return;

  auto contact = _addressbook_model->getContactById(contact_tmp.wallet_index);

  contact.auth_status = bts::addressbook::authorization_status::sent_request;
  _addressbook_model->storeContact(contact);

  emit authorizationStatus(contact.wallet_index);
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
  
  auto identity = ui->widget_Identity->currentIdentity();
  if (identity != nullptr)
  {
    bts::bitchat::private_contact_request_message request_msg;

    request_msg.from_first_name = identity->first_name;
    request_msg.from_last_name = identity->last_name;
    request_msg.from_keyhotee_id = identity->dac_id_string;
    request_msg.greeting_message = ui->message->toPlainText().toStdString();
    request_msg.from_channel = bts::network::channel_id(1);

    uint16_t request_param = ui->check_box_chat->isChecked();
    request_param |= ui->check_box_mail->isChecked() << 1;
    request_param |= ui->extend_public_key->isChecked() << 8;
    request_msg.request_param = request_param;
    request_msg.status = bts::bitchat::authorization_status::request;
    request_msg.recipient = ui->keyhoteeidpubkey->getPublicKey();

    genExtendedPubKey(request_msg.extended_pub_key);

    bts::addressbook::wallet_contact contact;
    if(Utils::matchContact(identity->public_key, &contact))
      request_msg.from_icon_png = contact.icon_png;

    _auth_processor.SendAuth(*identity, request_msg);

    setAuthorizationStatus();
  }
}

void RequestAuthorization::onStateWidget(KeyhoteeIDPubKeyWidget::CurrentState state)
{
  switch(state)
  {
    case KeyhoteeIDPubKeyWidget::CurrentState::InvalidData:
      ui->button_send->setEnabled(false);
      ui->add_contact->setEnabled(false);
      ui->add_contact->setChecked(false);
      break;
    case KeyhoteeIDPubKeyWidget::CurrentState::OkKeyhoteeID:
    case KeyhoteeIDPubKeyWidget::CurrentState::OkPubKey:
      ui->add_contact->setEnabled(true);
      ui->add_contact->setChecked(true);
      ui->button_send->setEnabled(true);
      checkAddAsNewContact();
      break;
    case KeyhoteeIDPubKeyWidget::CurrentState::IsStored:
      ui->add_contact->setEnabled(false);
      ui->add_contact->setChecked(false);
      ui->button_send->setEnabled(true);
      break;
    case KeyhoteeIDPubKeyWidget::CurrentState::IsIdentity:
      ui->add_contact->setEnabled(false);
      ui->add_contact->setChecked(false);
      ui->button_send->setEnabled(false);
      break;
    default:
      assert(false);
  }
}

void RequestAuthorization::done(int code)
{
  /// delete identity observer
  IdentityObservable::getInstance().deleteObserver(ui->widget_Identity);

  QDialog::done(code);
}
