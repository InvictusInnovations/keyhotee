#include "ContactView.hpp"
#include "ui_ContactView.h"
#include "AddressBookModel.hpp"
#include "public_key_address.hpp"

#include <KeyhoteeMainWindow.hpp>
#include <bts/application.hpp>
#include <bts/address.hpp>

#include <fc/thread/thread.hpp>
#include <fc/log/logger.hpp>

#include <QtWebKitWidgets/QWebFrame>
#include <QToolBar>
#include <QToolButton>
#include <QMessageBox>
#include <QFileDialog>

extern bool gMiningIsPossible;

bool ContactView::eventFilter(QObject* object, QEvent* event)
  {
  if (event->type() == QEvent::KeyPress)
    {
    QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
    switch (key_event->key())
      {
    case Qt::Key_Enter:
    case Qt::Key_Return:
      sendChatMessage();
      return true;
    default:
      break;
      }
    }
  return QObject::eventFilter(object, event);
  }

bool ContactView::isChatSelected()
  {
  return ui->contact_pages->currentWidget() == ui->chat_page;
  }

void ContactView::sendChatMessage()
  {
  auto msg = ui->chat_input->toPlainText();
  if (msg.size() != 0)
    {
    auto                               app = bts::application::instance();
    auto                               profile = app->get_profile();
    auto                               idents = profile->identities();
    bts::bitchat::private_text_message text_msg(msg.toStdString() );
    if (idents.size() )
      {
      fc::ecc::private_key my_priv_key = profile->get_keychain().get_identity_key(idents[0].dac_id_string);
      app->send_text_message(text_msg, _current_contact.public_key, my_priv_key);
      appendChatMessage("me", msg);
      }

    ui->chat_input->setPlainText(QString());
    }
  }

void ContactView::appendChatMessage(const QString& from, const QString& msg, const QDateTime& date_time) //DLNFIX2 improve formatting later
  {
  wlog("append... ${msg}", ("msg", msg.toStdString() ) );
  QString formatted_msg = date_time.toString("hh:mm ap") + " " + from + ": " + msg;
    #if 1
  QColor  color;
  if (from == "me")
    color = "grey";
  else
    color = "black";
  ui->chat_conversation->setTextColor(color);
  ui->chat_conversation->append(formatted_msg);
    #else //this doesn't start new paragraphs, probably not worth spending
  //time on as we'll like junk in favor of somethng else later
  QTextCursor text_cursor = ui->chat_conversation->textCursor();
  QString     colorName = (from == "me") ? "grey" : "black";
  formatted_msg = QString("<font color=\"%1\">%2</font>").arg(colorName).arg(formatted_msg);
  ui->chat_conversation->insertHtml(formatted_msg);
  cursor.movePosition(QTextCursor::NextBlock);
  ui->chat_conversation->setTextCursor(text_cursor);
    #endif
  }

ContactView::ContactView(QWidget* parent)
  : QWidget(parent),
  ui(new Ui::ContactView() )
  {
  _address_book = nullptr;
  _addingNewContact = false;
  ui->setupUi(this);
  setModyfied(false);
  _editing = false;
   message_tools = new QToolBar( ui->toolbar_container );
  QGridLayout* grid_layout = new QGridLayout(ui->toolbar_container);
  grid_layout->setContentsMargins(0, 0, 0, 0);
  grid_layout->setSpacing(0);
  ui->toolbar_container->setLayout(grid_layout);
  grid_layout->addWidget(message_tools, 0, 0);
  ui->email->setVisible (false);              //unsupported
  ui->phone->setVisible (false);
  ui->phone_label->setVisible (false);
  ui->email_label->setVisible (false);
  ui->privacy_comboBox->setVisible (false);
  ui->privacy_level_label->setVisible (false);//unsupported
  
  send_mail = new QAction( QIcon( ":/images/128x128/contact_info_send_mail.png"), tr("Mail"), this);
  edit_contact = new QAction( QIcon(":/images/128x128/contact_info_edit.png"), tr("Edit"), this);
  share_contact = new QAction(QIcon(":/images/read-icon.png"), tr("Share (need new icon)"), this);
  request_contact = new QAction( QIcon(":/images/128x128/contact_info_request_authorisation.png"), tr("Request authorisation"), this);
  save_contact = new QAction( QIcon(":/images/128x128/contact_info_save.png"), tr( "Save"), this );
  cancel_edit_contact = new QAction( QIcon(":/images/128x128/contact_info_cancel_edit.png"), tr("Discard changes"), this);
  connect(ui->icon_view, &QToolButton::clicked, this, &ContactView::onIconSearch);

  message_tools->addAction(cancel_edit_contact);
  message_tools->addAction(save_contact);  
  message_tools->addAction(edit_contact);
  message_tools->addAction(send_mail);  
  message_tools->addAction(share_contact);
  message_tools->addAction(request_contact);

  //ui->chat_conversation->setHtml( "<html><head></head><body>Hello World<br/></body></html>" );
  connect(save_contact, &QAction::triggered, this, &ContactView::onSave);
  connect(cancel_edit_contact, &QAction::triggered, this, &ContactView::onCancel);
  connect(edit_contact, &QAction::triggered, this, &ContactView::onEdit);
  connect(send_mail, &QAction::triggered, this, &ContactView::onMail);
  connect(share_contact, &QAction::triggered, this, &ContactView::onShareContact);
  connect(request_contact, &QAction::triggered, this, &ContactView::onRequestContact);

  connect(ui->firstname, &QLineEdit::textChanged, this, &ContactView::firstNameChanged);
  connect(ui->lastname, &QLineEdit::textChanged, this, &ContactView::lastNameChanged);
  connect(ui->id_edit, &QLineEdit::textChanged, this, &ContactView::keyhoteeIdChanged);
  connect(ui->id_edit, &QLineEdit::textEdited, this, &ContactView::keyhoteeIdEdited);
  connect(ui->public_key, &QLineEdit::textEdited, this, &ContactView::publicKeyEdited);
  connect(ui->public_key, &QLineEdit::textChanged, this, &ContactView::publicKeyChanged);
  connect( ui->privacy_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(privacyLevelChanged(int)));
  connect(ui->email, &QLineEdit::textChanged, this, &ContactView::emailChanged);
  connect(ui->phone, &QLineEdit::textChanged, this, &ContactView::phoneChanged);
  connect(ui->notes, &QPlainTextEdit::textChanged, this, &ContactView::notesChanged);
  connect(ui->public_key_to_clipboard, &QToolButton::clicked, this, &ContactView::onPublicKeyToClipboard);

  keyEdit(false);
  ui->chat_input->installEventFilter(this);

  setContact(Contact() );
  }

void ContactView::onEdit()
  {
  setAddingNewContact(false);   //editing  
  doDataExchange (false);
  keyEdit(true);   //and set focus on the first field
  ui->contact_pages->setCurrentIndex(info);
  ui->info_stack->setCurrentWidget(ui->info_edit);
  }

void ContactView::addNewContact ()
  {  
  doDataExchange (false);
  keyEdit(true);
  }

void ContactView::onSave()
  {
  try
    {
    if (doDataExchange (true))
      {
      if (_current_record)
        {
        //_current_contact.bit_id_hash = _current_record->name_hash;
        if (!_current_contact.public_key.valid() )
          {
          _current_contact.public_key = _current_record->active_key;
          FC_ASSERT(_current_contact.public_key.valid() );
          }
        // TODO: lookup block id / timestamp that registered this ID
        // _current_contact.known_since.setMSecsSinceEpoch( );
        }
      else if (!_current_record)  /// note: user is entering manual public key
        {
        elog("!current record??\n");
        /*
           if( _current_contact.known_since == QDateTime() )
           {
            _current_contact.known_since = QDateTime::currentDateTime();
           }
         */
        std::string enteredPKey = ui->public_key->text().toStdString();
        if (enteredPKey.empty() == false)
          {
          assert(public_key_address::is_valid(enteredPKey) && "Some bug in control validator");
          public_key_address key_address(enteredPKey);
          _current_contact.public_key = key_address.key;
          }
        }
      _current_contact.privacy_setting = bts::addressbook::secret_contact;
      _address_book->storeContact(_current_contact);

      keyEdit(false);
      }    
    }
  FC_RETHROW_EXCEPTIONS(warn, "onSave")
  }

void ContactView::onCancel()
  {
  setModyfied(false);
  if (isAddingNewContact())
    {
    this->setVisible(false);
    emit canceledAddContact();
    }
  else  //editing contact
    {    
    doDataExchange (false);
    keyEdit(false);
    }
  }

void ContactView::onChat()
  {
  ui->contact_pages->setCurrentIndex(chat);
  //clear unread message count on display of chat window
  //DLNFIX maybe getMainWindow can be removed via some connect magic or similar observer notification?
  ContactGui* contact_gui = GetKeyhoteeWindow()->getContactGui(_current_contact.wallet_index);
  if (contact_gui)
    contact_gui->setUnreadMsgCount(0);
  ui->chat_input->setFocus();
  }

void ContactView::onMail()
  {
  GetKeyhoteeWindow()->newMailMessageTo(_current_contact);
  }

void ContactView::onShareContact()
  {
  QMessageBox::warning(this, tr("Warning"), tr("Not supported"));
  }

void ContactView::onRequestContact()
  {
  QMessageBox::warning(this, tr("Warning"), tr("Not supported"));
  }

ContactView::~ContactView()
  {}

void ContactView::setContact(const Contact& current_contact)
  {
    _current_contact = current_contact;
    doDataExchange (false);
  }

Contact ContactView::getContact() const
  {
  return _current_contact;
  }

void ContactView::firstNameChanged(const QString& /*name*/)
  {
  setModyfied();
  }

void ContactView::keyhoteeIdChanged(const QString& /*name*/)
  {
  setModyfied();
  }

void ContactView::lastNameChanged(const QString& /*name*/)
  {
  setModyfied();
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
void ContactView::keyhoteeIdEdited(const QString& id)
  {
  /** TODO
     if( is_address( id ) )
     {
     _complete = true;
     }
     else
   */
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
  }

//implement real version and put in bitshares or fc (probably should be in fc)
bool is_registered_public_key(std::string public_key_string)
  {
  return false;  //(public_key_string == "invictus");
  }

void ContactView::publicKeyEdited(const QString& public_key_string)
  {
  ui->id_edit->clear();  //clear keyhotee id field
  if (gMiningIsPossible)
    lookupPublicKey();
  //check for validly hashed public key and enable/disable save button accordingly
  bool public_key_is_valid = public_key_address::is_valid(public_key_string.toStdString());
  bool doubleContact = false;
  if (public_key_is_valid)
    {
    if (! (doubleContact = existContactWithPublicKey (public_key_string.toStdString())))
      {
      ui->id_status->setText(tr("Public Key Only Mode: valid key") );
      ui->id_status->setStyleSheet("QLabel { color : green; }");
      }
    }
  else
    {
    ui->id_status->setText(tr("Public Key Only Mode: not a valid key") );
    ui->id_status->setStyleSheet("QLabel { color : red; }");
    }
  setValid (public_key_is_valid && ! doubleContact);
  }

void ContactView::lookupId()
  {
  try
    {
    auto current_id = ui->id_edit->text().toStdString();
    setValid (false);
    if (current_id.empty() )
      {
      ui->id_status->setText(QString() );
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
          setValid (true);
        }
      }
    else
      {
      ui->id_status->setStyleSheet("QLabel { color : red; }");
      ui->id_status->setText(tr("Unable to find ID") );
      ui->public_key->setText(QString());
      }
    }
  catch (const fc::exception& e)
    {
    ui->id_status->setText(e.to_string().c_str() );
    }
  }

void ContactView::lookupPublicKey()
  {
  std::string public_key_string = ui->public_key->text().toStdString();
  //fc::ecc::public_key public_key;
  //bts::bitname::client::reverse_name_lookup( public_key );
  bool        public_key_is_registered = is_registered_public_key(public_key_string);
  if (public_key_is_registered)
    ui->id_edit->setText("********");  //any better idea to indicate unknown but keyhoteeId?
  else
    ui->id_edit->setText(QString());  //clear keyhotee field if unregistered public key
  }

void ContactView::setAddressBook(AddressBookModel* addressbook)
  {
  _address_book = addressbook;
  }

AddressBookModel* ContactView::getAddressBook() const
  {
  return _address_book;
  }

void ContactView::onPublicKeyToClipboard()
  {
  QClipboard *clip = QApplication::clipboard();
  clip->setText(ui->public_key->text());
  }

void ContactView::keyEdit(bool enable)
  {
  _editing = enable;
  ui->firstname->setEnabled(enable);
  ui->lastname->setEnabled(enable);
  if (isAddingNewContact ())
    {
    //keyhoteeIds don't function when mining is not possible
    ui->id_edit->setEnabled(enable && gMiningIsPossible);
    ui->public_key->setEnabled(enable);
    }
    else
    {
    /// note: you cannot change the id of a contact once it has been
    /// set... you must create a new contact anytime their public key
    /// changes.
    ui->id_edit->setEnabled(false);
    ui->public_key->setEnabled(false);
    }
   
  ui->privacy_comboBox->setEnabled(enable);
  ui->email->setEnabled(enable);
  ui->phone->setEnabled(enable);
  ui->notes->setEnabled(enable);
  ui->icon_view->setEnabled(enable);

  ui->id_status->setVisible(enable);
  ui->keyhotee_founder->setVisible(!enable && _current_contact.getAge() == 1);
  save_contact->setEnabled(enable);
  cancel_edit_contact->setEnabled(enable);
  send_mail->setEnabled(!enable);
  edit_contact->setEnabled(!enable);
  share_contact->setEnabled(false);
  request_contact->setEnabled(false);

  ui->contact_pages->setTabEnabled(chat, !enable);

  if (enable)
    {
    ui->firstname->setFocus();
    setModyfied(false);
    }
  }

bool ContactView::CheckSaving()
  {
  if (isEditing() && isModyfied())
    {
    QMessageBox::StandardButton ret;
    ret = QMessageBox::question(this, tr("Application"),
                                tr("The contact has been modified.\nDo you want to save your changes ?"),
                                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (ret == QMessageBox::Cancel)
      {
      ui->firstname->setFocus();
      return false;
      }
    else
      {
      if (ret == QMessageBox::Yes)
        {
        if (isValid())
          {
          onSave();
          }
        else
          {
          ui->id_edit->setFocus();
          ui->id_edit->selectAll();
          QMessageBox::warning(this, tr("Application"),
                               tr("Please correct the Keyhotee ID."),
                               QMessageBox::Ok);
          return false;
          }
        }
      else
        {
        onCancel();
        }
      setModyfied(false);
      }
    }
  else if (isEditing() && !isModyfied())
    {
    keyEdit(false);
    }
  return true;
  }

void ContactView::setValid(bool valid)
  {
  _validForm = valid;
  save_contact->setEnabled(valid && isEditing());
  }

void ContactView::onIconSearch()
  {
  auto writableLocation = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
  auto fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), writableLocation,
                                               tr("Image Files (*.png *.jpg *.bmp)"));
  if (!fileName.isEmpty())
    ui->icon_view->setIcon(QIcon(fileName));
  }


bool ContactView::doDataExchange (bool valid)
  {
  if (! valid)
    {
    if (isAddingNewContact ()) 
      {
      setValid(false);
      ui->firstname->setText("");
      ui->lastname->setText("");
      ui->id_edit->setText("");
      ui->icon_view->setIcon( QIcon(":/images/user.png") );
      ui->notes->setPlainText("");
      ui->email->setText("");
      ui->phone->setText("");
      ui->public_key->setText ("");
      ui->privacy_comboBox->setCurrentIndex (0);      
      ui->contact_pages->setCurrentIndex(info);

      if (gMiningIsPossible)
        ui->id_status->setText(tr("Please provide a valid ID or public key") );
      else
        ui->id_status->setText(tr("Public Key Only Mode") );
      }
    else 
      {
      setValid(true);
      /** TODO... restore this kind of check
         if( _current_contact.bit_id_public_key != _current_contact.public_key  )
         {
         ui->id_status->setText(
                  tr( "Warning! Keyhotee ID %1 no longer matches known public key!" ).arg(_current_contact.bit_id) );
         }
       */
      ui->firstname->setText( _current_contact.first_name.c_str() );
      ui->lastname->setText( _current_contact.last_name.c_str() );
      ui->id_edit->setText( _current_contact.dac_id_string.c_str() );      
      ui->icon_view->setIcon( _current_contact.getIcon() );
      ui->notes->setPlainText( _current_contact.notes.c_str() );
      //ui->email->setText( _current_contact.email_address );
      //ui->phone->setText( _current_contact.phone_number );
      //privacy_comboBox
      std::string public_key_string = public_key_address( _current_contact.public_key );
      ui->public_key->setText( public_key_string.c_str() );
      ui->keyhotee_founder->setVisible(!_editing && _current_contact.getAge() == 1);
      ui->id_status->setText(QString());
      }
    }
    else
    {
    _current_contact.first_name     = ui->firstname->text().toStdString();
    _current_contact.last_name      = ui->lastname->text().toStdString();
    _current_contact.dac_id_string  = ui->id_edit->text().toStdString();
    _current_contact.setIcon (ui->icon_view->icon ());
    _current_contact.notes  = ui->notes->toPlainText().toStdString();
    //_current_contact.email_address = ui->email->text().toStdString();
    //_current_contact.phone_number = ui->phone->text().toStdString();
    //privacy_comboBox
    }
    return true;
  }


bool ContactView::existContactWithPublicKey (const std::string& public_key_string)
{
   std::string my_public_key = public_key_address( _current_contact.public_key );
   if (public_key_string != my_public_key)
   {
      auto addressbook = bts::get_profile()->get_addressbook();
      if(! public_key_string.empty())
      {
         public_key_address key_address(public_key_string);
         auto findContact = addressbook->get_contact_by_public_key( key_address.key );
         if (findContact)
         {
            ui->id_status->setText( tr("This contact is already added to the list") );
            ui->id_status->setStyleSheet("QLabel { color : red; }");
            return true;
         }
      }     
   }
   return false;
}
