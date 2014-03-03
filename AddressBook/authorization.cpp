#include "authorization.hpp"
#include "ui_authorization.h"

#include "AddressBookModel.hpp"
#include "Contact.hpp"
#include "public_key_address.hpp"
#include "utils.hpp"

#include <fc/reflect/variant.hpp>

#include <QMessageBox>
#include <QToolBar>

Authorization::Authorization(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::Authorization)
{
  ui->setupUi(this);

  _address_book = nullptr;
  _owner_item = nullptr;

  ui->keyhoteeidpubkey->setMode(KeyhoteeIDPubKeyWidget::ModeWidget::RequestAuthorization);
  ui->keyhoteeidpubkey->setEditable(false);
  ui->keyhoteeidpubkey->showCopyToClipboard(false);

  _toolbar = new QToolBar(ui->toolbar_container);
  QGridLayout* grid_layout = new QGridLayout(ui->toolbar_container);
  grid_layout->setContentsMargins(0, 0, 0, 0);
  grid_layout->setSpacing(0);
  ui->toolbar_container->setLayout(grid_layout);
  grid_layout->addWidget(_toolbar, 0, 0);
  
  _accept = new QAction( QIcon( ":/images/request_accept.png"), tr("Accept"), this);
  _deny = new QAction( QIcon( ":/images/request_deny.png"), tr("Deny"), this);  
  _block = new QAction( QIcon(":/images/request_block.png"), tr("Block"), this);
  
  _toolbar->addAction(_accept);  
  _toolbar->addAction(_deny);
  _toolbar->addAction(_block);

  QLabel *label = new QLabel((tr("     Response for authorization request")));
  _toolbar->addWidget (label);
  QFont font;  
  font.setBold(true);
  font.setPointSize (16);
  label->setFont (font);

  connect(_accept, &QAction::triggered, this, &Authorization::onAccept);
  connect(_deny, &QAction::triggered, this, &Authorization::onDeny);
  connect(_block, &QAction::triggered, this, &Authorization::onBlock);
  connect(ui->add_contact , &QGroupBox::toggled, this, &Authorization::onAddAsNewContact);
  connect(ui->keyhoteeidpubkey, &KeyhoteeIDPubKeyWidget::currentState, this, &Authorization::onStateWidget);

  // setting the background color of the frame so that the window looked like a window "create new contact"
  QPalette palette = ui->frame->palette();
  palette.setColor(backgroundRole(), QGuiApplication::palette().base().color());
  ui->frame->setPalette(palette);
}

Authorization::~Authorization()
{
  delete ui;
}

void Authorization::setAddressBook(AddressBookModel* addressbook)
{
  _address_book = addressbook;
}

void Authorization::setMsg(const TPublicKey& sender, const TRequestMessage& msg)
{
  _from_pub_key = sender;
  _reqmsg = msg;

  std::string public_key_string = public_key_address(_from_pub_key.serialize());
  ui->keyhoteeidpubkey->setPublicKey(public_key_string.c_str());
  ui->keyhoteeidpubkey->setKeyhoteeID(_reqmsg.from_keyhotee_id.c_str());

  ui->first_name->setText(_reqmsg.from_first_name.c_str() );
  ui->last_name->setText(_reqmsg.from_last_name.c_str() );
  
  ui->check_box_chat->setChecked(_reqmsg.request_param & 0x01);
  ui->check_box_mail->setChecked(_reqmsg.request_param>>1 & 0x01);
  ui->extend_public_key->setChecked(_reqmsg.request_param>>8 & 0x01);

  ui->message->setText(_reqmsg.greeting_message.c_str());
}

void Authorization::setOwnerItem(AuthorizationItem* item)
{
  _owner_item = item;
}

void Authorization::processResponse()
{
  if(_reqmsg.status == TAuthorizationStatus::accept)
    acceptExtendedPubKey();

  setAuthorizationStatus(_reqmsg.status);
}

void Authorization::onAccept()
{
  addAsNewContact();
  acceptExtendedPubKey();
  sendReply(TAuthorizationStatus::accept);
  setAuthorizationStatus(TAuthorizationStatus::accept);
  close();
  emit itemAcceptRequest(_owner_item);
}

void Authorization::onDeny()
{
  sendReply(TAuthorizationStatus::deny);
  setAuthorizationStatus(TAuthorizationStatus::deny);
  close();
  emit itemDenyRequest(_owner_item);
}

void Authorization::onBlock()
{
  sendReply(TAuthorizationStatus::block);
  setAuthorizationStatus(TAuthorizationStatus::block);
  close();
  emit itemBlockRequest(_owner_item);
}

void Authorization::addAsNewContact()
{
  if(ui->add_contact->isEnabled() && ui->add_contact->isChecked())
  {
    Contact new_conntact;
    new_conntact.first_name       = ui->first_name->text().toStdString();
    new_conntact.last_name        = ui->last_name->text().toStdString();
    new_conntact.dac_id_string    = ui->keyhoteeidpubkey->getKeyhoteeID().toStdString();     // can better directly from the message?
    new_conntact.public_key       = _from_pub_key;
    new_conntact.privacy_setting  = bts::addressbook::secret_contact;
    new_conntact.setIcon(QIcon(":/images/user.png"));

    _address_book->storeContact(new_conntact);
  }
}

void Authorization::acceptExtendedPubKey()
{
  if(ui->extend_public_key->isChecked())
  {
    auto addressbook = bts::get_profile()->get_addressbook();
    TWalletContact contact;
    if(!Utils::matchContact(_from_pub_key, &contact))
      return;
    contact.send_trx_address = _reqmsg.extended_pub_key;
    addressbook->store_contact(contact);
  }
}

void Authorization::genExtendedPubKey(std::string identity_dac_id, TExtendPubKey &extended_pub_key)
{
  if(ui->extend_public_key->isChecked())
  {
    auto profile = bts::get_profile();
    TWalletContact contact;
    if(!Utils::matchContact(_from_pub_key, &contact))
      return;

    extended_pub_key = profile->get_keychain().get_public_account(identity_dac_id, contact.wallet_index);
  }
}

void Authorization::sendReply(TAuthorizationStatus status)
{
  TWalletContact my_identity;
  if(!Utils::matchContact(_reqmsg.recipient, &my_identity))
    return;

  auto app = bts::application::instance();
  auto profile = app->get_profile();
  bts::bitchat::private_contact_request_message request_msg;

  request_msg.from_first_name = my_identity.first_name;
  request_msg.from_last_name = my_identity.last_name;
  request_msg.from_keyhotee_id = my_identity.dac_id_string;
  request_msg.greeting_message = "";
  request_msg.from_channel = bts::network::channel_id(1);
    
  uint16_t request_param = ui->check_box_chat->isChecked();
  request_param |= ui->check_box_mail->isChecked() << 1;
  request_param |= ui->extend_public_key->isChecked() << 8;
  request_msg.request_param = request_param;
  
  request_msg.status = status;
  request_msg.recipient = _from_pub_key;

  if(status == TAuthorizationStatus::accept)
    if(ui->extend_public_key->isChecked())
      genExtendedPubKey(my_identity.dac_id_string, request_msg.extended_pub_key);

  fc::ecc::private_key my_priv_key = profile->get_keychain().get_identity_key(my_identity.dac_id_string);
  app->send_contact_request(request_msg, ui->keyhoteeidpubkey->getPublicKey(), my_priv_key);
}

void Authorization::setAuthorizationStatus(TAuthorizationStatus status)
{
  auto addressbook = bts::get_profile()->get_addressbook();
  TWalletContact contact;
  if(!Utils::matchContact(_from_pub_key, &contact))
    return;

  switch(status)
  {
    case TAuthorizationStatus::accept:
      contact.auth_status = TContAuthoStatus::accepted;
      break;
    case TAuthorizationStatus::block:
      contact.auth_status = TContAuthoStatus::blocked;
      break;
    case TAuthorizationStatus::deny:
      contact.auth_status = TContAuthoStatus::denied;
      break;
    default:
      contact.auth_status = TContAuthoStatus::unauthorized;
      assert(false);
  }
  addressbook->store_contact(contact);
}

void Authorization::onAddAsNewContact(bool checked)
{
  if(ui->extend_public_key->isChecked() &&
      ui->add_contact->isEnabled() && !ui->add_contact->isChecked())
  {
    QMessageBox::information(this, tr("Information"), tr("To accept the Extended Public Key is necessary to add a new contact."));
    ui->add_contact->setChecked(true);
  }
}

void Authorization::onStateWidget(KeyhoteeIDPubKeyWidget::CurrentState state)
{
  switch(state)
  {
    case KeyhoteeIDPubKeyWidget::CurrentState::InvalidData:
    case KeyhoteeIDPubKeyWidget::CurrentState::IsStored:
      ui->add_contact->setEnabled(false);
      break;
    case KeyhoteeIDPubKeyWidget::CurrentState::OkKeyhoteeID:
    case KeyhoteeIDPubKeyWidget::CurrentState::OkPubKey:
      ui->add_contact->setEnabled(true);
      break;
    default:
      assert(false);
  }
}

