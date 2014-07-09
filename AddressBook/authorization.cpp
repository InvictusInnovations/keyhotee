#include "authorization.hpp"
#include "ui_authorization.h"

#include "AddressBookModel.hpp"
#include "Contact.hpp"
#include "public_key_address.hpp"
#include "utils.hpp"

#include <fc/reflect/variant.hpp>
#include <fc/log/logger.hpp>

#include <QMessageBox>
#include <QToolBar>


Authorization::Authorization(IAuthProcessor& auth_processor, AddressBookModel* addressbook_model,
                  const TRequestMessage& msg, const THeaderStoredMsg& header, QWidget *parent) :
  QWidget(parent),
  _auth_processor(auth_processor)
{
  _addressbook_model = addressbook_model;
  _reqmsg = msg;
  _header = header;
}

Authorization::~Authorization()
{
}

void Authorization::processResponse()
{
  if(_reqmsg.status == TAuthorizationStatus::accept)
    acceptExtendedPubKey();

  setAuthorizationStatus(_reqmsg.status);

  TWalletIdentity my_identity;
  if(!Utils::matchIdentity(_reqmsg.recipient, &my_identity))
    return;

  _auth_processor.storeAuthorization(my_identity, _reqmsg, _header);
}

void Authorization::acceptExtendedPubKey() const
{
  if(_reqmsg.request_param>>8 & 0x01)   // if Exchange of extended public keys
  {
    TWalletContact contact_tmp;
    if(!Utils::matchContact(_header.from_key, &contact_tmp))
      return;

    auto contact = _addressbook_model->getContactById(contact_tmp.wallet_index);

    contact.send_trx_address = _reqmsg.extended_pub_key;
    _addressbook_model->storeContact(contact);
  }
}

void Authorization::setAuthorizationStatus(TAuthorizationStatus status)
{
  TWalletContact contact_tmp;
  if(!Utils::matchContact(_header.from_key, &contact_tmp))
    return;

  auto contact = _addressbook_model->getContactById(contact_tmp.wallet_index);

  switch(status)
  {
    case TAuthorizationStatus::accept:
      switch(_reqmsg.request_param & 0x03)
      {
        case 1:   // chat
          contact.auth_status = TContAuthoStatus::accepted_chat;
          break;
        case 2:   // mail
          contact.auth_status = TContAuthoStatus::accepted_mail;
          break;
        case 3:   // chat and mail
          contact.auth_status = TContAuthoStatus::accepted;
          break;
      }
      break;
    case TAuthorizationStatus::block:
      if(_reqmsg.status == TAuthorizationStatus::request)
        contact.auth_status = TContAuthoStatus::i_block;
      else
        contact.auth_status = TContAuthoStatus::blocked_me;
      break;
    case TAuthorizationStatus::deny:
      contact.auth_status = TContAuthoStatus::denied;
      break;
    default:
      contact.auth_status = TContAuthoStatus::unauthorized;
      assert(false);
  }
  _addressbook_model->storeContact(contact);

  emit authorizationStatus(contact.wallet_index);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///                                    AuthorizationView                                        ///
///////////////////////////////////////////////////////////////////////////////////////////////////

AuthorizationView::AuthorizationView(IAuthProcessor& auth_processor, AddressBookModel* addressbook_model,
                    const TRequestMessage& msg, const THeaderStoredMsg& header) :
  ui(new Ui::AuthorizationView),
  Authorization(auth_processor, addressbook_model, msg, header)
{
  ui->setupUi(this);

  _owner_item = nullptr;

  ui->keyhoteeidpubkey->setMode(KeyhoteeIDPubKeyWidget::ModeWidget::AuthorizationView);
  ui->keyhoteeidpubkey->setEditable(false);
  ui->keyhoteeidpubkey->hideCopyKeysToClipboard();

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

  connect(_accept, &QAction::triggered, this, &AuthorizationView::onAccept);
  connect(_deny, &QAction::triggered, this, &AuthorizationView::onDeny);
  connect(_block, &QAction::triggered, this, &AuthorizationView::onBlock);
  connect(ui->add_contact , &QGroupBox::toggled, this, &AuthorizationView::onAddAsNewContact);
  connect(ui->keyhoteeidpubkey, &KeyhoteeIDPubKeyWidget::currentState, this, &AuthorizationView::onStateWidget);

  // setting the background color of the frame so that the window looked like a window "create new contact"
  QPalette palette = ui->frame->palette();
  palette.setColor(backgroundRole(), QGuiApplication::palette().base().color());
  ui->frame->setPalette(palette);

  _from_pub_key = header.from_key;

  std::string public_key_string = public_key_address(_from_pub_key.serialize());
  ui->keyhoteeidpubkey->setPublicKey(public_key_string.c_str());
  ui->keyhoteeidpubkey->setKeyhoteeID(_reqmsg.from_keyhotee_id.c_str());
  ui->keyhoteeidpubkey->setAddressBook(_addressbook_model);

  ui->first_name->setText(_reqmsg.from_first_name.c_str() );
  ui->last_name->setText(_reqmsg.from_last_name.c_str() );
  
  ui->check_box_chat->setChecked(_reqmsg.request_param & 0x01);
  ui->check_box_mail->setChecked(_reqmsg.request_param>>1 & 0x01);
  ui->extend_public_key->setChecked(_reqmsg.request_param>>8 & 0x01);

  ui->message->setText(_reqmsg.greeting_message.c_str());
}

AuthorizationView::~AuthorizationView()
{
  delete ui;
}

void AuthorizationView::setOwnerItem(AuthorizationItem* item)
{
  _owner_item = item;
}

void AuthorizationView::showEvent(QShowEvent * event)
{
  std::string public_key_string = public_key_address(_from_pub_key.serialize());
  ui->keyhoteeidpubkey->setPublicKey(public_key_string.c_str());
  ui->keyhoteeidpubkey->setKeyhoteeID(_reqmsg.from_keyhotee_id.c_str());
}

void AuthorizationView::onAccept()
{
  addAsNewContact();
  acceptExtendedPubKey();
  sendReply(TAuthorizationStatus::accept);
  setAuthorizationStatus(TAuthorizationStatus::accept);
  close();
  emit itemAcceptRequest(_owner_item);
}

void AuthorizationView::onDeny()
{
  sendReply(TAuthorizationStatus::deny);
  setAuthorizationStatus(TAuthorizationStatus::deny);
  close();
  emit itemDenyRequest(_owner_item);
}

void AuthorizationView::onBlock()
{
  addAsNewContact();
  sendReply(TAuthorizationStatus::block);
  setAuthorizationStatus(TAuthorizationStatus::block);
  close();
  emit itemBlockRequest(_owner_item);
}

void AuthorizationView::addAsNewContact()
{
  if(ui->add_contact->isEnabled() && ui->add_contact->isChecked())
  {
    bts::addressbook::wallet_contact contact_tmp;
    contact_tmp.first_name = ui->first_name->text().toStdString();
    contact_tmp.last_name = ui->last_name->text().toStdString();
    contact_tmp.dac_id_string = _reqmsg.from_keyhotee_id;
    contact_tmp.public_key = _from_pub_key;
    contact_tmp.privacy_setting = bts::addressbook::secret_contact;
    if(_reqmsg.from_icon_png)
      contact_tmp.icon_png = *_reqmsg.from_icon_png;

    Contact new_conntact = Contact(contact_tmp);
    _addressbook_model->storeContact(new_conntact);
  }
}

void AuthorizationView::genExtendedPubKey(std::string identity_dac_id, TExtendPubKey &extended_pub_key)
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

void AuthorizationView::sendReply(TAuthorizationStatus status)
{
  TWalletIdentity my_identity;
  if(!Utils::matchIdentity(_reqmsg.recipient, &my_identity))
    return;

  if(status != TAuthorizationStatus::deny)
  {
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

    _auth_processor.SendAuth(my_identity, request_msg);
  }

  _auth_processor.storeAuthorization(my_identity, _reqmsg, _header);
}

void AuthorizationView::onAddAsNewContact(bool checked)
{
  if(ui->extend_public_key->isChecked() &&
      ui->add_contact->isEnabled() && !ui->add_contact->isChecked())
  {
    QMessageBox::information(this, tr("Information"), tr("To accept the Extended Public Key is necessary to add a new contact."));
    ui->add_contact->setChecked(true);
  }
}

void AuthorizationView::onStateWidget(KeyhoteeIDPubKeyWidget::CurrentState state)
{
  switch(state)
  {
    case KeyhoteeIDPubKeyWidget::CurrentState::InvalidData:
    case KeyhoteeIDPubKeyWidget::CurrentState::IsStored:
    case KeyhoteeIDPubKeyWidget::CurrentState::IsIdentity:
      ui->add_contact->setEnabled(false);
      ui->add_contact->setChecked(false);
      break;
    case KeyhoteeIDPubKeyWidget::CurrentState::OkKeyhoteeID:
    case KeyhoteeIDPubKeyWidget::CurrentState::OkPubKey:
      ui->add_contact->setEnabled(true);
      ui->add_contact->setChecked(true);
      break;
    default:
      assert(false);
  }
}
