#include "keyhoteeidpubkeywidget.hpp"
#include "ui_keyhoteeidpubkeywidget.h"

#include "AddressBookModel.hpp"
#include "public_key_address.hpp"

#include <bts/application.hpp>
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
  connect(ui->keyhotee_id, &QLineEdit::textChanged, this, &KeyhoteeIDPubKeyWidget::keyhoteeIdChanged);
  connect(ui->keyhotee_id, &QLineEdit::textEdited, this, &KeyhoteeIDPubKeyWidget::keyhoteeIdEdited);
  connect(ui->public_key, &QLineEdit::textChanged, this, &KeyhoteeIDPubKeyWidget::publicKeyChanged);
  connect(ui->public_key, &QLineEdit::textEdited, this, &KeyhoteeIDPubKeyWidget::publicKeyEdited);
}

KeyhoteeIDPubKeyWidget::~KeyhoteeIDPubKeyWidget()
{
    delete ui;
}

void KeyhoteeIDPubKeyWidget::setKeyhoteeID(const QString& keyhotee_id)
{
  ui->keyhotee_id->setText(keyhotee_id);
  keyhoteeIdEdited(keyhotee_id);
}

void KeyhoteeIDPubKeyWidget::setPublicKey(const QString& public_key_string)
{
  ui->public_key->setText(public_key_string);
  publicKeyEdited(public_key_string);
}

void KeyhoteeIDPubKeyWidget::keyhoteeIdChanged(const QString& /*name*/)
{
}

void KeyhoteeIDPubKeyWidget::publicKeyChanged(const QString& name)
{
}


/*****************  Algorithm for handling keyhoteeId, keyhoteeeId status, and public key fields
   Notes:
   If gMiningIsPossible,
   We can lookup a public key from a kehoteeId
   We can validate a public key is registered,
    but we can't lookup the associated keyhoteeId, only a hash of the keyhoteeId

   Some choices in Display Status for id not found on block chain: Available, Unable to find, Not registered

 *** When creating new wallet_identity (this is for later implementation and some details may change):

   Note: Public key field is not editable (only keyhotee-generated public keys are allowed as they must be tied to wallet)

   If gMiningPossible,
   Display Mining Effort combo box:
    options: Let Expire, Renew Quarterly, Renew Monthly, Renew Weekly, Renew Daily, Max Effort

   If keyhoteeId changed, lookup id and report status
    Display status: Not Available (red), Available (black), Registered To Me (green), Registering (yellow)
        (If keyhoteeId is not registered and mining effort is not 'Let Expire', then status is "Registering')
    OR:
    Display status: Registered (red), Not Registered (black), Registered To Me (green), Registering (yellow)
        (If keyhoteeId is not registered and mining effort is not 'Let Expire', then status is "Registering')
    Generate new public key based on keyhoteeId change and display it


   If not gMiningPossible,
   Hide Mining Effort combo box:

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
   if keyhotee set, set as not editable
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
  _last_validate = fc::time_point::now();
  ui->id_status->setText(tr("Looking up id...") );

  fc::async( [ = ](){
      fc::usleep(fc::microseconds(500 * 1000) );
      if (fc::time_point::now() > (_last_validate + fc::microseconds(500 * 1000)))
        lookupId();
    }
    );
}

//implement real version and put in bitshares or fc (probably should be in fc) ***************
bool is_registered_public_key_(std::string public_key_string)
{
  return false;  //(public_key_string == "invictus");
}

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
    std::string current_id = fc::trim(ui->keyhotee_id->text().toUtf8().constData());
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
      ui->id_status->setText(tr("Unable to find ID") );
      ui->public_key->setText(QString());
      emit currentState(InvalidData);
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
}

void KeyhoteeIDPubKeyWidget::onPublicKeyToClipboard()
{
  QClipboard *clip = QApplication::clipboard();
  clip->setText(ui->public_key->text());
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
            ui->id_status->setText( tr("This contact is already added to the list") );
            ui->id_status->setStyleSheet("QLabel { color : green; }");
            emit currentState(IsStored);
            return true;
          default:
            assert(false);
        }
      }
    }     
  }
  return false;
}

void KeyhoteeIDPubKeyWidget::showCopyToClipboard(bool visible)
{
  ui->public_key_to_clipboard->setVisible(visible);
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
      ui->keyhotee_id->setEnabled(true);
      ui->keyhotee_id->setReadOnly(!editable);
      ui->public_key->setReadOnly(!editable);
      break;
    default:
      assert(false);
  }
}

bool KeyhoteeIDPubKeyWidget::event(QEvent *e)
{
  if (e->type() == QEvent::Show)
  {
    if(ui->keyhotee_id->text().size() > 0)
      setKeyhoteeID(ui->keyhotee_id->text());
    else if(ui->public_key->text().size() > 0)
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

