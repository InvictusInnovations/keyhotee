#include "ContactView.hpp"
#include "ui_ContactView.h"

#include "AddressBookModel.hpp"
#include "Contact.hpp"
#include "ContactGui.hpp"
#include "KeyhoteeMainWindow.hpp"
#include "public_key_address.hpp"
#include "RequestAuthorization.hpp"

#include "Identity/IdentityObservable.hpp"

#include <fc/thread/thread.hpp>
#include <fc/log/logger.hpp>

#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QToolBar>
#include <QToolButton>

extern bool gMiningIsPossible;

#ifdef ALPHA_RELEASE
void display_founder_key_status(const QString& keyhotee_id, const QString& key, QLabel* status_label);
#endif



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
  if (getKeyhoteeWindow()->getConnectionProcessor()->GetPeerConnectionCount() == 0)
  {
    QMessageBox::warning(this, tr("Application"), 
      tr("No connection to send chat message."), QMessageBox::Ok);
    
    return;
  }

  auto msg = ui->chat_input->toPlainText();
  if (msg.isEmpty())
  {
    return;
  }
    
  auto identity = ui->widget_Identity->currentIdentity();
  if (identity != nullptr)
  {
    auto                               app = bts::application::instance();
    auto                               profile = app->get_profile();
    bts::bitchat::private_text_message text_msg(msg.toUtf8().constData());

    fc::ecc::private_key my_priv_key = profile->get_keychain().get_identity_key(identity->dac_id_string);
    app->send_text_message(text_msg, _current_contact.public_key, my_priv_key);
    appendChatMessage("me", msg);
  }

  ui->chat_input->setPlainText(QString());
}

void ContactView::appendChatMessage(const QString& from, const QString& msg, const QDateTime& date_time) //DLNFIX2 improve formatting later
{
  wlog("append... ${msg}", ("msg", msg.toUtf8().constData() ) );
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
  ui(new Ui::ContactView() ),
  _validForm(false)
{
  _address_book = nullptr;  
  ui->setupUi(this);
  _addingNewContact = false;
  _editing = false;
  _modified = false;

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
  //default contact view: info page
  ui->contact_pages->setCurrentIndex(info);
  
  /// add identity observer
  IdentityObservable::getInstance().addObserver( ui->widget_Identity );

  send_mail = new QAction( QIcon( ":/images/128x128/contact_info_send_mail.png"), tr("Mail"), this);
  chat_contact = new QAction( QIcon( ":/images/chat.png"), tr("Chat"), this);  
  edit_contact = new QAction( QIcon(":/images/128x128/contact_info_edit.png"), tr("Edit"), this);
  share_contact = new QAction(QIcon(":/images/128x128/contact_share.png"), tr("Share contact"), this);
  request_contact = new QAction( QIcon(":/images/128x128/contact_info_request_authorisation.png"), tr("Request authorisation"), this);
  save_contact = new QAction( QIcon(":/images/128x128/contact_info_save.png"), tr( "Save"), this );
  cancel_edit_contact = new QAction( QIcon(":/images/128x128/contact_info_cancel_edit.png"), tr("Discard changes"), this);
  connect(ui->icon_view, &QToolButton::clicked, this, &ContactView::onIconSearch);

  message_tools->addAction(send_mail);  
  message_tools->addAction(chat_contact);
  message_tools->addAction(share_contact);
  message_tools->addAction(request_contact);
  separatorToolBar = message_tools->addSeparator ();
  message_tools->addAction(edit_contact);
  message_tools->addAction(save_contact);  
  message_tools->addAction(cancel_edit_contact);

  QLabel *label = new QLabel((tr("     Create new contact")));
  label_createContact = message_tools->addWidget (label);
  QFont font;  
  font.setBold(true);
  font.setPointSize (16);
  label->setFont (font);

  ui->khid_pubkey->setMode(KeyhoteeIDPubKeyWidget::ModeWidget::ShowContact);

  connect(save_contact, &QAction::triggered, this, &ContactView::onSave);
  connect(cancel_edit_contact, &QAction::triggered, this, &ContactView::onCancel);
  connect(edit_contact, &QAction::triggered, this, &ContactView::onEdit);
  connect(send_mail, &QAction::triggered, this, &ContactView::onMail);
  connect(chat_contact, &QAction::triggered, this, &ContactView::onChat);
  connect(share_contact, &QAction::triggered, this, &ContactView::onShareContact);
  connect(request_contact, &QAction::triggered, this, &ContactView::onRequestContact);

  connect(ui->firstname, &QLineEdit::textChanged, this, &ContactView::firstNameChanged);
  connect(ui->lastname, &QLineEdit::textChanged, this, &ContactView::lastNameChanged);
  connect( ui->privacy_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(privacyLevelChanged(int)));
  connect(ui->email, &QLineEdit::textChanged, this, &ContactView::emailChanged);
  connect(ui->phone, &QLineEdit::textChanged, this, &ContactView::phoneChanged);
  connect(ui->notes, &QPlainTextEdit::textChanged, this, &ContactView::notesChanged);
  connect(ui->sendButton, &QPushButton::clicked, this, &ContactView::onSend);
  connect(ui->chat_input, &QPlainTextEdit::textChanged, this, &ContactView::onTextChanged);

  connect(ui->contact_pages, &QTabWidget::currentChanged, this, &ContactView::currentTabChanged);

  connect(ui->mining_effort_slider, &QSlider::valueChanged, this, &ContactView::onMiningEffortSliderChanged);
  connect(ui->khid_pubkey, &KeyhoteeIDPubKeyWidget::currentState, this, &ContactView::onStateWidget);

  contactEditable(false);
  ui->chat_input->installEventFilter(this);

  checkSendMailButton();
  setContact(Contact() );
}

ContactView::~ContactView()
{
  IdentityObservable::getInstance().deleteObserver( ui->widget_Identity );
  delete ui;
}

void ContactView::onEdit()
{
  setAddingNewContact(false);   //editing  
  ToDialog();
  contactEditable(true);   //and set focus on the first field
  ui->contact_pages->setCurrentIndex(info);
  ui->info_stack->setCurrentWidget(ui->info_edit);
}

void ContactView::setAddingNewContact(bool addNew)
{
  _addingNewContact = addNew;
  if(addNew)
    ui->khid_pubkey->setMode(KeyhoteeIDPubKeyWidget::ModeWidget::AddContact);
  else
    ui->khid_pubkey->setMode(KeyhoteeIDPubKeyWidget::ModeWidget::ShowContact);
}

void ContactView::addNewContact ()
{  
  ToDialog();
  contactEditable(true);
}

void ContactView::setPublicKey(const QString& public_key_string)
{
  ui->khid_pubkey->setPublicKey(public_key_string);
}

void ContactView::onSave()
{
  try
  {
    if (FromDialog())
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
        std::string enteredPKey = ui->khid_pubkey->getPublicKeyText().toStdString();
        if (enteredPKey.empty() == false)
        {
          bool publicKeySemanticallyValid = false;
          assert(public_key_address::is_valid(enteredPKey, &publicKeySemanticallyValid));
          assert(publicKeySemanticallyValid);
          public_key_address key_address(enteredPKey);
          _current_contact.public_key = key_address.key;
        }
      }
      _current_contact.privacy_setting = bts::addressbook::secret_contact;
      int idxNewContact = _address_book->storeContact(_current_contact);

      /// notify identity observers
      if (_current_contact.isOwn())
        IdentityObservable::getInstance().notify(_current_contact);

      contactEditable(false);
      emit savedNewContact(idxNewContact);
    }    
  }
  FC_RETHROW_EXCEPTIONS(warn, "onSave")
}

void ContactView::onCancel()
{
  setModified(false);
  ToDialog();
  contactEditable(false);
  if (isAddingNewContact())
  {
    emit canceledNewContact();
  }
  else  //editing contact
  {    
  }
}

void ContactView::onInfo()
  {
  ui->contact_pages->setCurrentIndex(info);
  }

void ContactView::onChat()
{
  ui->contact_pages->setCurrentIndex(chat);
  //clear unread message count on display of chat window
  //DLNFIX maybe getMainWindow can be removed via some connect magic or similar observer notification?
  ContactGui* contact_gui = getKeyhoteeWindow()->getContactGui(_current_contact.wallet_index);
  if (contact_gui)
    contact_gui->setUnreadMsgCount(0);
  ui->chat_input->setFocus();
}

void ContactView::onMail()
{
  getKeyhoteeWindow()->newMailMessageTo(_current_contact);
}

void ContactView::onShareContact()
{
  QList<const Contact*> contacts;
  contacts.push_back(&_current_contact);

  getKeyhoteeWindow()->shareContact(contacts);
}

void ContactView::onRequestContact()
{
  RequestAuthorization *request = new RequestAuthorization(this,
    *getKeyhoteeWindow()->getConnectionProcessor(), _address_book);
  connect(request, &RequestAuthorization::authorizationStatus, getKeyhoteeWindow(), &KeyhoteeMainWindow::onUpdateAuthoStatus);

  if(ui->khid_pubkey->getKeyhoteeID().isEmpty() == false && gMiningIsPossible)
    request->setKeyhoteeID(_current_contact.dac_id_string.c_str());
  else
  {
    std::string public_key_string = public_key_address(_current_contact.public_key.serialize());
    request->setPublicKey(public_key_string.c_str());
  }
  request->enableAddContact(false);
  request->show();
}

void ContactView::setContact(const Contact& current_contact)
{
    _current_contact = current_contact;
    ui->khid_pubkey->setContact(_current_contact);
    ToDialog();
}

Contact ContactView::getContact() const
{
  return _current_contact;
}

void ContactView::firstNameChanged(const QString& /*name*/)
{
  setModified();
}

void ContactView::lastNameChanged(const QString& /*name*/)
{
  setModified();
}

void ContactView::checkSendMailButton()
{
    auto app = bts::application::instance();
    auto profile = app->get_profile();

    auto idents = profile->identities();
    send_mail->setEnabled(idents.size() > 0);

    request_contact->setEnabled(idents.size() > 0 && !_current_contact.isOwn() && !isEditing());
}

void ContactView::setAddressBook(AddressBookModel* addressbook)
{
  _address_book = addressbook;
  ui->khid_pubkey->setAddressBook(addressbook);
}

AddressBookModel* ContactView::getAddressBook() const
{
  return _address_book;
}

void ContactView::contactEditable(bool enable)
{
  _editing = enable;
  ui->firstname->setEnabled(enable);
  ui->lastname->setEnabled(enable);
  ui->khid_pubkey->setEditable(enable);
  setEnabledSaveContact ();
  if (isAddingNewContact ())
  {
    send_mail->setVisible(false);
    chat_contact->setVisible(false);
    edit_contact->setVisible(false);
    share_contact->setVisible(false);
    request_contact->setVisible(false);
    separatorToolBar->setVisible(false);
    label_createContact->setVisible(true);
  }
  else
  {
    send_mail->setVisible(true);
    chat_contact->setVisible(true);
    edit_contact->setVisible(true);
    share_contact->setVisible(true);
    request_contact->setVisible(true);
    separatorToolBar->setVisible(true);
    label_createContact->setVisible(false);
  }
   
  ui->privacy_comboBox->setEnabled(enable);
  ui->email->setEnabled(enable);
  ui->phone->setEnabled(enable);
  ui->notes->setEnabled(enable);
  ui->icon_view->setEnabled(enable);

  ui->keyhotee_founder->setVisible(!enable && _current_contact.isKeyhoteeFounder());
  bool is_owner = _current_contact.isOwn();
  ui->keyhoteeID_status->setVisible(!enable && is_owner);
  ui->authorization_status->setVisible(!enable && !is_owner);
  ui->mining_effort_slider->setDisabled(!enable && is_owner);
  
  cancel_edit_contact->setEnabled(enable);
  send_mail->setEnabled(!enable);
  chat_contact->setEnabled(!enable);
  edit_contact->setEnabled(!enable);
  share_contact->setEnabled(!enable);
  request_contact->setEnabled(!enable && !is_owner);

  ui->contact_pages->setTabEnabled(chat, !enable);

  if (enable)
  {
    ui->firstname->setFocus();
    setModified(false);
  }
  checkSendMailButton();
}

bool ContactView::CheckSaving()
{
  if (isEditing() && isModified())
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
      setModified(false);
    }
  }
  else if (isEditing() && !isModified())
  {
    onCancel();
  }
  return true;
}

void ContactView::setValid(bool valid)
{
  _validForm = valid;
  setEnabledSaveContact ();
}

void ContactView::onIconSearch()
{
  auto writableLocation = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
  auto fileName = QFileDialog::getOpenFileName(this, tr("Select Avatar image for contact"), writableLocation,
                                               tr("Image Files (*.png *.jpg *.bmp)"));

  if (!fileName.isEmpty())
  {
    save_contact->setEnabled(_validForm);
    ui->icon_view->setIcon(QIcon(fileName));
  }
}

void ContactView::ToDialog()
{
  if (isAddingNewContact ()) 
  {
    setValid(false);
    ui->firstname->setText("");
    ui->lastname->setText("");
    ui->khid_pubkey->setKeyhoteeID("");
    ui->icon_view->setIcon( QIcon(":/images/user.png") );
    ui->notes->setPlainText("");
    ui->email->setText("");
    ui->phone->setText("");
    ui->khid_pubkey->setPublicKey("");
    ui->privacy_comboBox->setCurrentIndex (0);
    ui->contact_pages->setCurrentIndex(info);
  }
  else 
  {
    setValid(true);

    refreshDialog(_current_contact);

    bool is_owner = _current_contact.isOwn();
    ui->keyhoteeID_status->setVisible(!_editing && is_owner);
    ui->authorization_status->setVisible(!_editing && !is_owner);
    ui->mining_effort->setVisible(is_owner);
    ui->mining_effort_slider->setVisible(is_owner);
    ui->mining_effort_slider->setDisabled(!_editing);
    ui->mining_effort_label->setVisible(is_owner);
    ui->mining_effort_label_2->setVisible(is_owner);
  }
  setEnabledSaveContact();
}

bool ContactView::FromDialog()
{
    _current_contact.first_name     = ui->firstname->text().toUtf8().constData();
    _current_contact.last_name      = ui->lastname->text().toUtf8().constData();
    _current_contact.dac_id_string  = ui->khid_pubkey->getKeyhoteeID().toUtf8().constData();
    _current_contact.setIcon (ui->icon_view->icon());
    _current_contact.notes  = ui->notes->toPlainText().toUtf8().constData();
    //_current_contact.email_address = ui->email->text().toStdString();
    //_current_contact.phone_number = ui->phone->text().toStdString();
    //privacy_comboBox
    setEnabledSaveContact();
    return true; //currently no way to have bad data, but original coding allowed it, so I've left this for now
}

void ContactView::checkKeyhoteeIdStatus()
{
  ui->keyhotee_founder->setVisible(!_editing && _current_contact.isKeyhoteeFounder());
  bool is_owner = _current_contact.isOwn();
  if(is_owner)
  {
#ifdef ALPHA_RELEASE
    QString keyhotee_id = ui->khid_pubkey->getKeyhoteeID();
    QString founder_code = _current_contact.notes.c_str();
    display_founder_key_status(keyhotee_id,founder_code,ui->keyhoteeID_status);
#else
    ui->mining_effort_slider->setValue( static_cast<int>(_current_contact.getMiningEffort()));
    //if registered keyhoteeId
    auto name_record = bts::application::instance()->lookup_name(_current_contact.dac_id_string);
    if (name_record)
    {
      //  if keyhoteeId's public key matches ours.
      if (name_record->active_key == _current_contact.public_key)
      { //Registered to us
        ui->keyhoteeID_status->setStyleSheet("QLabel { background-color : green; color : black; }");
        ui->keyhoteeID_status->setText(tr("Registered"));
      }
      else //Not Available (someone else owns it)
      {
        ui->keyhoteeID_status->setStyleSheet("QLabel { background-color : red; color : black; }");
        ui->keyhoteeID_status->setText(tr("Not Available"));
      }
    }
    else //Unregistered (no one has it yet)
    {
      ui->keyhoteeID_status->setStyleSheet("QLabel { background-color : yellow; color : black; }");
      ui->keyhoteeID_status->setText(tr("Unregistered"));
    }
  #endif
  } //if (is_owner)
}

void ContactView::setEnabledSaveContact ()
{
  save_contact->setEnabled(_validForm && isEditing() && isModified());
}

void ContactView::setModified(bool modified)
{
  if (isEditing())
  {
    _modified = modified;
    setEnabledSaveContact ();
    }
  }

void ContactView::currentTabChanged(int index)
  {
  if (index == chat)
    onChat ();
  }

void ContactView::onTextChanged()
{
    if (ui->chat_input->toPlainText().length() > _max_chat_char)
    {
        QString text = ui->chat_input->toPlainText();
        text.chop(text.length() - _max_chat_char);
        ui->chat_input->setPlainText(text);

        QTextCursor cursor = ui->chat_input->textCursor();
        cursor.setPosition(ui->chat_input->document()->characterCount() - 1);
        ui->chat_input->setTextCursor(cursor);
    }
}

void ContactView::onSend ()
  {
  sendChatMessage();
  ui->chat_input->setFocus ();
  }

void ContactView::onMiningEffortSliderChanged(int mining_effort)
  {
  _current_contact.setMiningEffort(mining_effort);
  //DLN right now this is set in identity object, not in contact, so it's immediate and
  //non-cancelable edit, eventually we probably should somehow fix this.
  setModified();
  }

void ContactView::onStateWidget(KeyhoteeIDPubKeyWidget::CurrentState state)
{
  setModified();

  switch(state)
  {
    case KeyhoteeIDPubKeyWidget::CurrentState::InvalidData:
      setValid(false);
      break;
    case KeyhoteeIDPubKeyWidget::CurrentState::OkKeyhoteeID:
    case KeyhoteeIDPubKeyWidget::CurrentState::OkPubKey:
      setValid(true);
      break;
    case KeyhoteeIDPubKeyWidget::CurrentState::IsStored:
    case KeyhoteeIDPubKeyWidget::CurrentState::IsIdentity:
      break;
    default:
      assert(false);
  }
}

void ContactView::setContactFromvCard(const Contact &contact)
{
  refreshDialog(contact);

  setModified();
}

void ContactView::checkAuthorizationStatus()
{
  bts::addressbook::authorization_status status;

  try
  {
    auto contact = bts::get_profile()->get_addressbook()->get_contact_by_public_key(_current_contact.public_key);
    if(contact)
      status = contact->auth_status;
    else
      status = bts::addressbook::authorization_status::unauthorized;
  }
  catch (const fc::exception&)
  {
    status = bts::addressbook::authorization_status::unauthorized;
  }

  _current_contact.auth_status = status;

  if(status == bts::addressbook::authorization_status::sent_request)
  {
    ui->authorization_status->setText( tr("Request sent") );
  }
  else if(status == bts::addressbook::authorization_status::accepted)
  {
    ui->authorization_status->setText( tr("Authorized") );
  }
  else if(status == bts::addressbook::authorization_status::accepted_chat)
  {
    ui->authorization_status->setText(tr("Authorized chat"));
  }
  else if(status == bts::addressbook::authorization_status::accepted_mail)
  {
    ui->authorization_status->setText(tr("Authorized mail"));
  }
  else if(status == bts::addressbook::authorization_status::denied)
  {
    ui->authorization_status->setText( tr("Denied") );
  }
  else if(status == bts::addressbook::authorization_status::i_block)
  {
    ui->authorization_status->setText( tr("Blocked") );
  }
  else if(status == bts::addressbook::authorization_status::blocked_me)
  {
    ui->authorization_status->setText(tr("Blocked me"));
  }
  else
  {
    ui->authorization_status->setText( tr("Unauthorized") );
  }
}

void ContactView::refreshDialog(const Contact &contact)
{
  /** TODO... restore this kind of check
    if( _current_contact.bit_id_public_key != _current_contact.public_key  )
    {
      ui->id_status->setText(
                  tr( "Warning! Keyhotee ID %1 no longer matches known public key!" ).arg(_current_contact.bit_id) );
    }
    */
  ui->firstname->setText( contact.first_name.c_str() );
  ui->lastname->setText( contact.last_name.c_str() );

  //set public key from contact record initially
  std::string public_key_string = public_key_address( contact.public_key );
  ui->khid_pubkey->setPublicKey( public_key_string.c_str() );
  //set keyhoteeID from contact record, this may override public key from contact record
  //if mining is enabled and ID is registered in blockchain
  ui->khid_pubkey->setKeyhoteeID(contact.dac_id_string.c_str());

  ui->icon_view->setIcon( contact.getIcon() );
  ui->notes->setPlainText( contact.notes.c_str() );
  //ui->email->setText( _current_contact.email_address );
  //ui->phone->setText( _current_contact.phone_number );
  //privacy_comboBox
  //DLNFIX TODO: add check to see if we are synced on blockchain. If not synched,
  //             display "Keyhotee ledger not accessible"
  checkKeyhoteeIdStatus();
  checkAuthorizationStatus();
}