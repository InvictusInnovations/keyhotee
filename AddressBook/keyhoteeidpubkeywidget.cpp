#include <iostream>

#include "keyhoteeidpubkeywidget.hpp"
#include "ui_keyhoteeidpubkeywidget.h"

#include "AddressBookModel.hpp"
#include "public_key_address.hpp"

#include <bts/application.hpp>
#include <bts/keychain.hpp>
#include <fc/thread/thread.hpp>

#include <QClipboard>

extern bool gMiningIsPossible;

KeyhoteeIDPubKeyWidget::KeyhoteeIDPubKeyWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::KeyhoteeIDPubKeyWidget)
{
  ui->setupUi(this);
  _address_book = nullptr;
  
  _my_mode = ShowContact;

  connect(ui->public_key_to_clipboard, &QToolButton::clicked, this, &KeyhoteeIDPubKeyWidget::onPublicKeyToClipboard);
  connect(ui->keyhotee_id, &QLineEdit::textEdited, this, &KeyhoteeIDPubKeyWidget::keyhoteeIdEdited);
  connect(ui->public_key, &QLineEdit::textEdited, this, &KeyhoteeIDPubKeyWidget::publicKeyEdited);
}

KeyhoteeIDPubKeyWidget::~KeyhoteeIDPubKeyWidget()
  {
  cancelLookupThread();

  delete ui;
  }

void KeyhoteeIDPubKeyWidget::setKeyhoteeID(const QString& keyhotee_id)
{
  ui->keyhotee_id->setText(keyhotee_id);
  if (_my_mode != AuthorizationView && 
      /// Keyhotee ID and public key you can not edit
      /// so donn't call keyhoteeIdEdited
      _my_mode != ShowContact)
    keyhoteeIdEdited(keyhotee_id);
}

void KeyhoteeIDPubKeyWidget::setPublicKey(const QString& public_key_string)
{
  ui->public_key->setText(public_key_string);
  if (_my_mode == AuthorizationView ||
      /// Keyhotee ID and public key you can not edit
      /// so donn't call keyhoteeIdEdited
      _my_mode == ShowContact)
  {
    existContactWithPublicKey(public_key_string.toStdString());
  }
  else
    publicKeyEdited(public_key_string);
}

/*****************  Algorithm for handling keyhoteeId, keyhoteeeId status, and public key fields
   Notes:
   If gMiningIsPossible,
   We can lookup a public key from a kehoteeId
   We can validate a public key is registered,
    but we can't lookup the associated keyhoteeId, only a hash of the keyhoteeId

   Some choices in Display Status for id not found on block chain: Available, Unable to find, Not registered

 *** When creating new wallet_identity:

   Note: Public key field is not editable 
   (only keyhotee-generated public keys are allowed as they must be tied to wallet)

   If gMiningPossible,
   Display Mining Effort combo box:
    options for later?: Let Expire, Renew Quarterly, Renew Monthly, Renew Weekly, Renew Daily, Max Effort
    for now, use 0-100% slider

   If keyhoteeId changed, lookup id and report status
    Display status: Not Available (red), Available (black), Registered To Me (green), Registering (yellow)
        (If keyhoteeId is not registered and mining effort is not 'Let Expire', then status is "Registering')
    OR:
    Display status: Registered (red), Not Registered (black), Registered To Me (green), Registering (yellow)
        (If keyhoteeId is not registered and mining effort is not 'Let Expire', then status is "Registering')
    Generate new public key based on keyhoteeId change and display it


   If not gMiningPossible,
     Grey out Mining Effort combo box:
     If keyhoteeId changed, just keep it
       Generate new public key based on keyhoteeId change and display it

 *** When adding a contact:

   If gMiningPossible,
   If keyhoteeId changed, lookup id and report status
    Display status: Registered (green), Unable to find (red)
    if keyhoteeId registered in block chain, set public key field to display it
    if keyhoteeId field not registered in block chain, clear public key field
    enable save if valid public key or disable save

   If public key changed, validate it
    if public key is registered, change keyhotee field to ********
    if public key is not registered, clear keyhotee id field
    enable save if valid public key or disable save

   If not gMiningPossible,
   Disable keyhoteeId field
   If public key changed, validate it
    enable save if valid public key or disable save

 *** When editing a contact:

   If gMiningPossible,
   Public key is not editable
   if keyhoteeId set, set as not editable
   If keyhoteeId blank, lookup id and report status
    Display status: Matches (green)
                    Mismatch (red)
   Doesn't save keyhoteeId on mismatch (i.e. field data isn't transferred to the contact record)

   If not gMiningPossible,
   Public key is not editable
   KeyhoteeId is not editable

 */

void KeyhoteeIDPubKeyWidget::keyhoteeIdEdited(const QString& keyhotee_id)
{
  if (gMiningIsPossible)
  {
  cancelLookupThread();
  ui->id_status->setText(tr("Looking up id...") );
  _lookupThreadState = fc::async([=]() {lookupId();});
  }
}

//implement real version and put in bitshares or fc (probably should be in fc) ***************
bool is_registered_public_key_(std::string public_key_string)
{
  return false;  //(public_key_string == "invictus");
}

//TODO decide whether this code and keyhoteeIdEdited can be moved to Changed versions instead
void KeyhoteeIDPubKeyWidget::publicKeyEdited(const QString& public_key_string)
{
  ui->keyhotee_id->clear();  //clear keyhotee id field
  if (gMiningIsPossible)
    lookupPublicKey();
  //check for validly hashed public key and enable/disable save button accordingly
  bool publicKeySemanticallyValid = false;
  bool public_key_is_valid = public_key_address::is_valid(public_key_string.toStdString(), 
                                                          &publicKeySemanticallyValid);

  ui->public_key->setStyleSheet("QLineEdit { color : black; }");

  bool doubleContact = false;
  if (public_key_is_valid)
  {
    if (!publicKeySemanticallyValid)
    {
      ui->public_key->setStyleSheet("QLineEdit { color : red; }");
      emit currentState(InvalidData);
    }
    else if (! (doubleContact = existContactWithPublicKey(public_key_string.toStdString())))
    {
      ui->id_status->setText(tr("Public Key Only Mode: valid key") );
      ui->id_status->setStyleSheet("QLabel { color : green; }");
      emit currentState(OkPubKey);
    }
  }
  else
  {
    emit currentState(InvalidData);
    if(public_key_string.isEmpty())
    {
      ui->id_status->setStyleSheet("QLabel { color : black; }");
      if (gMiningIsPossible)
        ui->id_status->setText(tr("Please provide a valid ID or public key") );
      else
        ui->id_status->setText(tr("Public Key Only Mode") );
    }
    else
    {
      ui->id_status->setText(tr("Public Key Only Mode: not a valid key") );
      ui->id_status->setStyleSheet("QLabel { color : red; }");
    }
  }

}

void KeyhoteeIDPubKeyWidget::lookupId()
{
  try
  {
    bool isOwn = _current_contact.isOwn();
    if (isOwn)
    { //if own identity, we don't change the public key!
      //DLN add some code here to show status (once status shows in GUI)
    }
    else //regular contact (not identity)
    { //all this code needs double checking and rethinking, but first lets get UI
      //correct (right now id_status is replicated in ContactView as keyhoteeID_status!)
      std::string current_id = fc::trim(ui->keyhotee_id->text().toStdString());
      emit currentState(InvalidData);
      if (current_id.empty() )
      {
        ui->id_status->setText(QString() );
        emit currentState(InvalidData);
        return;
      }
      _current_record = bts::application::instance()->lookup_name(current_id);
      if (_current_record)
      {
        ui->id_status->setStyleSheet("QLabel { color : green; }");
        ui->id_status->setText(tr("Registered") );
        std::string public_key_string = public_key_address(_current_record->active_key);
        ui->public_key->setText(public_key_string.c_str() );
        if (_address_book != nullptr)
        {
          if (! existContactWithPublicKey (public_key_string))
          {
            emit currentState(OkKeyhoteeID);
          }
        }
      }
      else
      {
        ui->id_status->setStyleSheet("QLabel { color : red; }");
        ui->id_status->setText(tr("Unregistered") );
        ui->public_key->setText(QString());
        emit currentState(InvalidData);
      }
    }
  }
  catch (const fc::exception& e)
  {
    ui->id_status->setText(e.to_string().c_str() );
  }
}

void KeyhoteeIDPubKeyWidget::lookupPublicKey()
{
  std::string public_key_string = ui->public_key->text().toStdString();
  //fc::ecc::public_key public_key;
  //bts::bitname::client::reverse_name_lookup( public_key );
  bool        public_key_is_registered = is_registered_public_key_(public_key_string);
  if(public_key_is_registered)
    ui->keyhotee_id->setText("********");  //any better idea to indicate unknown but keyhoteeId?
  else
    ui->keyhotee_id->setText(QString());  //clear keyhotee field if unregistered public key
}

void KeyhoteeIDPubKeyWidget::setAddressBook(AddressBookModel* address_book)
{
  _address_book = address_book;
}

void KeyhoteeIDPubKeyWidget::setContact(const Contact& current_contact)
{
    _current_contact = current_contact;
    bool isOwner = _current_contact.isOwn();
    //public key of identity is not directly editable
    ui->keyhotee_id->setText( _current_contact.dac_id_string.c_str() );
    std::string public_key_string = public_key_address(_current_contact.public_key.serialize());
    ui->public_key->setText(public_key_string.c_str());
    ui->public_key->setEnabled( !isOwner );
    ui->private_key_to_clipboard->setVisible(isOwner);
}

void KeyhoteeIDPubKeyWidget::onPublicKeyToClipboard()
{
  QClipboard *clip = QApplication::clipboard();
  clip->setText(ui->public_key->text());
}

inline
bool KeyhoteeIDPubKeyWidget::isLookupThreadActive() const
  {
  return (_lookupThreadState.valid() && _lookupThreadState.ready() == false);
  }

void KeyhoteeIDPubKeyWidget::cancelLookupThread()
  {
  if(isLookupThreadActive())
    {
    _lookupThreadState.cancel_and_wait();
    }
  }

bool KeyhoteeIDPubKeyWidget::existContactWithPublicKey (const std::string& public_key_string)
{
  std::string my_public_key = public_key_address( _current_contact.public_key );
  if (public_key_string != my_public_key)
  {
    auto addressbook = bts::get_profile()->get_addressbook();
    if(! public_key_string.size()==0)
    {
      public_key_address key_address(public_key_string);
      auto findContact = addressbook->get_contact_by_public_key( key_address.key );
      if (findContact)
      {
        switch(_my_mode)
        {
          case ModeWidget::ShowContact:
          case ModeWidget::AddContact:
            ui->id_status->setText( tr("This contact is already added to the list") );
            ui->id_status->setStyleSheet("QLabel { color : red; }");
            return true;
          case ModeWidget::RequestAuthorization:
            if(Utils::isOwnedPublicKey(getPublicKey()))
            {
              ui->id_status->setText(tr("This contact is your identity"));
              ui->id_status->setStyleSheet("QLabel { color : red; }");
              emit currentState(IsIdentity);
              return true;
            }
          case ModeWidget::AuthorizationView:
            ui->id_status->setText( tr("Public Key Only Mode: valid key") ); //tr("This contact is already added to the list") );
            ui->id_status->setStyleSheet("QLabel { color : green; }");
            emit currentState(IsStored);
            return true;
          default:
            assert(false);
        }
      }
      if(_my_mode == ModeWidget::AuthorizationView)
        emit currentState(OkPubKey);
    }
  }
  return false;
}

void KeyhoteeIDPubKeyWidget::hideCopyKeysToClipboard()
{
  ui->public_key_to_clipboard->setVisible(false);
  ui->private_key_to_clipboard->setVisible(false);
}

void KeyhoteeIDPubKeyWidget::setEditable(bool editable)
{
  switch(_my_mode)
  {
    case ModeWidget::ShowContact:
      /// note: you cannot change the id of a contact once it has been
      /// set... you must create a new contact anytime their public key
      /// changes.
      ui->keyhotee_id->setEnabled(false);
      ui->public_key->setReadOnly(true);
      ui->id_status->setVisible(false);
      break;
    case ModeWidget::AddContact:
      ui->keyhotee_id->setReadOnly(false);
      ui->keyhotee_id->setEnabled(editable && gMiningIsPossible);
      ui->public_key->setReadOnly(!editable);
      ui->id_status->setVisible(editable);
      break;
    case ModeWidget::RequestAuthorization:
      ui->keyhotee_id->setEnabled(editable && gMiningIsPossible);
      ui->keyhotee_id->setReadOnly(!editable);
      ui->public_key->setReadOnly(!editable);
      break;
    case ModeWidget::AuthorizationView:
      ui->keyhotee_id->setEnabled(false);
      ui->keyhotee_id->setReadOnly(true);
      ui->public_key->setReadOnly(true);
      ui->id_status->setVisible(false);
      break;
    default:
      assert(false);
  }
}

bool KeyhoteeIDPubKeyWidget::event(QEvent *e)
{
  if (e->type() == QEvent::Show)
  {
    if(ui->keyhotee_id->text().isEmpty() == false)
      setKeyhoteeID(ui->keyhotee_id->text());
    else if(ui->public_key->text().isEmpty() == false)
      setPublicKey(ui->public_key->text());
  }
  return QWidget::event(e);
}

fc::ecc::public_key KeyhoteeIDPubKeyWidget::getPublicKey()
{
  public_key_address key_address(ui->public_key->text().toStdString());
  return fc::ecc::public_key(key_address.key);
}

QString KeyhoteeIDPubKeyWidget::getPublicKeyText()
{
  return ui->public_key->text();
}

QString KeyhoteeIDPubKeyWidget::getKeyhoteeID()
{
  return ui->keyhotee_id->text();
}

void KeyhoteeIDPubKeyWidget::setFocus(Qt::FocusReason reason)
{
  ui->keyhotee_id->setFocus(reason);
}
void KeyhoteeIDPubKeyWidget::on_private_key_to_clipboard_clicked()
{
  //Performing the WIF conversion inline, since I don't see bts::utilities in this repository.
  //If this is available somewhere in this repository, please change it.
  fc::sha256 secret = bts::application::instance()->get_profile()->get_keychain().get_identity_key(_current_contact.dac_id_string).priv_key;
  const size_t size_of_data_to_hash = sizeof(secret) + 1;
  const size_t size_of_hash_bytes = 4;
  char data[size_of_data_to_hash + size_of_hash_bytes];
  data[0] = (char)0x80;
  memcpy(&data[1], (char*)&secret, sizeof(secret));
  fc::sha256 digest = fc::sha256::hash(data, size_of_data_to_hash);
  digest = fc::sha256::hash(digest);
  memcpy(data + size_of_data_to_hash, (char*)&digest, size_of_hash_bytes);

  QApplication::clipboard()->setText(fc::to_base58(data, sizeof(data)).c_str());
}
