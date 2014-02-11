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
  ui(new Ui::KeyhoteeIDPubKeyWidget),
  _validForm(false)
{
  ui->setupUi(this);
  _address_book = nullptr;

  connect(ui->public_key_to_clipboard, &QToolButton::clicked, this, &KeyhoteeIDPubKeyWidget::onPublicKeyToClipboard);
  connect(ui->keyhotee_id, &QLineEdit::textEdited, this, &KeyhoteeIDPubKeyWidget::keyhoteeIdEdited);
  connect(ui->public_key, &QLineEdit::textEdited, this, &KeyhoteeIDPubKeyWidget::publicKeyEdited);
}

KeyhoteeIDPubKeyWidget::~KeyhoteeIDPubKeyWidget()
{
    delete ui;
}

void KeyhoteeIDPubKeyWidget::onPublicKeyToClipboard()
{
  QClipboard *clip = QApplication::clipboard();
  clip->setText(ui->public_key->text());
}

void KeyhoteeIDPubKeyWidget::lookupId()
{
  try
  {
    std::string current_id = fc::trim(ui->keyhotee_id->text().toUtf8().constData());
    setValid (false);
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
//      if (_address_book != nullptr)
//      {
        if (! existContactWithPublicKey (public_key_string))
        {
          emit currentState(OkKeyhoteeID);
          setValid (true);
        }
//      }
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

//implement real version and put in bitshares or fc (probably should be in fc)
bool is_registered_public_key_(std::string public_key_string)
{
  return false;  //(public_key_string == "invictus");
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

void KeyhoteeIDPubKeyWidget::setKeyhoteeID(const QString& keyhotee_id)
{
  ui->keyhotee_id->setText(keyhotee_id);
  keyhoteeIdEdited(keyhotee_id);
  //setModifed();
}

void KeyhoteeIDPubKeyWidget::setPublicKey(const QString& public_key_string)
{
  ui->public_key->setText(public_key_string);
  publicKeyEdited(public_key_string);
  //setModyfied();
}

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
    ui->id_status->setText(tr("Public Key Only Mode: not a valid key") );
    ui->id_status->setStyleSheet("QLabel { color : red; }");
    emit currentState(InvalidData);
  }
  setValid (public_key_is_valid && ! doubleContact && publicKeySemanticallyValid);
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

void KeyhoteeIDPubKeyWidget::setValid(bool valid)
{
  _validForm = valid;
}

void KeyhoteeIDPubKeyWidget::showCopyToClipboard(bool visible)
{
  ui->public_key_to_clipboard->setVisible(visible);
}

void KeyhoteeIDPubKeyWidget::setEditable(bool editable)
{
  ui->keyhotee_id->setReadOnly(!editable);
  ui->public_key->setReadOnly(!editable);
}

fc::ecc::public_key KeyhoteeIDPubKeyWidget::getPublicKey()
{
  public_key_address key_address(ui->public_key->text().toStdString());
  return fc::ecc::public_key(key_address.key);
}

