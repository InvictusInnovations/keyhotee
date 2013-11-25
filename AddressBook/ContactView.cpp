#include "ContactView.hpp"
#include "ui_ContactView.h"
#include "AddressBookModel.hpp"

#include <KeyhoteeMainWindow.hpp>
#include <bts/application.hpp>
#include <bts/address.hpp>

#include <fc/thread/thread.hpp>
#include <fc/log/logger.hpp>

#include <fc/crypto/base58.hpp>
#include <fc/crypto/city.hpp>
#include <fc/crypto/elliptic.hpp>
#include <QWebFrame>

extern bool gMiningIsPossible;

struct public_key_address
{
    fc::ecc::public_key_data key;
    uint32_t                 check;

    bool is_valid()const
    {
       return uint32_t(fc::city_hash64( (char*)&key, sizeof(key) )) == check;
    }
    public_key_address(){}

    public_key_address( const fc::ecc::public_key_data& k )
    {
       key = k;
       check =  uint32_t(fc::city_hash64( (char*)&key, sizeof(key) ));
    }
    public_key_address( const std::string& str )
    {
       std::vector<char> bin = fc::from_base58( str );
       if( bin.size() == 37 )
       {
          memcpy( (char*)&key, bin.data(), 33 );
          memcpy( (char*)&check, bin.data()+33, 4 );
       }
    }
    operator std::string()const
    {
        fc::array<char,37> data;
        memcpy( (char*)&data, (char*)&key, 33 );
        memcpy( ((char*)&data)+33, (char*)&check, 4 );
        return fc::to_base58( (char*)&data, 37 );
    }
};

bool ContactView::eventFilter(QObject* object, QEvent* event)
{
  if (event->type() == QEvent::KeyPress) 
  {
     QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
     switch(key_event->key()) 
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
    return (ui->contact_pages->currentWidget() == ui->chat_page);
}


void ContactView::sendChatMessage()
{
    auto msg = ui->chat_input->toPlainText();
    if( msg.size() != 0 )
    {
        auto app = bts::application::instance();
        auto profile = app->get_profile();
        auto  idents = profile->identities();
        bts::bitchat::private_text_message text_msg( msg.toStdString() );
        if( idents.size() )
        {
           fc::ecc::private_key my_priv_key = profile->get_keychain().get_identity_key( idents[0].dac_id );
           app->send_text_message( text_msg, _current_contact.public_key, my_priv_key );
           appendChatMessage( "me", msg );
        }

        ui->chat_input->setPlainText(QString());
    }
}
void ContactView::appendChatMessage( const QString& from, const QString& msg, const QDateTime& date_time )
{ //DLNFIX2 improve formatting later
    wlog( "append... ${msg}", ("msg",msg.toStdString() ) );
    QString formatted_msg = date_time.toString("hh:mm ap") + " "+ from + ": " + msg;
    #if 1
    QColor color;
    if (from == "me")
      color = "grey";
    else
      color = "black";
    ui->chat_conversation->setTextColor(color);
    ui->chat_conversation->append(formatted_msg);
    #else //this doesn't start new paragraphs, probably not worth spending
    //time on as we'll like junk in favor of somethng else later
    QTextCursor text_cursor = ui->chat_conversation->textCursor();
    QString colorName = (from == "me") ? "grey" : "black";
    formatted_msg = QString("<font color=\"%1\">%2</font>").arg(colorName).arg(formatted_msg);
    ui->chat_conversation->insertHtml(formatted_msg);
    cursor.movePosition(QTextCursor::NextBlock);
    ui->chat_conversation->setTextCursor(text_cursor);
    #endif
}


ContactView::ContactView( QWidget* parent )
: QWidget(parent),
  ui( new Ui::ContactView() )
{
   _address_book = nullptr;
   ui->setupUi(this);
   //ui->chat_conversation->setHtml( "<html><head></head><body>Hello World<br/></body></html>" );
   connect( ui->save_button, &QPushButton::clicked, this, &ContactView::onSave );
   connect( ui->cancel_button, &QPushButton::clicked, this, &ContactView::onCancel );
   connect( ui->edit_button, &QPushButton::clicked, this, &ContactView::onEdit );
   connect( ui->mail_button, &QAbstractButton::clicked, this, &ContactView::onMail );
   connect( ui->chat_button, &QAbstractButton::clicked, this, &ContactView::onChat );
   connect( ui->info_button, &QAbstractButton::clicked, this, &ContactView::onInfo );

   connect( ui->firstname, &QLineEdit::textChanged, this, &ContactView::firstNameChanged );
   connect( ui->lastname, &QLineEdit::textChanged, this, &ContactView::lastNameChanged );
   connect( ui->id_edit, &QLineEdit::textEdited, this, &ContactView::keyhoteeIdEdited );
   connect( ui->public_key, &QLineEdit::textEdited, this, &ContactView::publicKeyEdited );

   ui->chat_input->installEventFilter(this);

   setContact( Contact() );

}

void ContactView::onEdit()
{
    ui->contact_pages->setCurrentWidget( ui->info_page );
    ui->info_stack->setCurrentWidget(ui->info_edit);
}

void ContactView::onSave()
{ try {
    _current_contact.first_name     = ui->firstname->text().toStdString();
    _current_contact.last_name      = ui->lastname->text().toStdString();
    _current_contact.dac_id_string  = ui->id_edit->text().toStdString();
    if( _current_record )
    {
       //_current_contact.bit_id_hash = _current_record->name_hash;
       if( !_current_contact.public_key.valid() )
       {
            _current_contact.public_key = _current_record->pub_key;
            FC_ASSERT( _current_contact.public_key.valid() );
       }
       // TODO: lookup block id / timestamp that registered this ID
       // _current_contact.known_since.setMSecsSinceEpoch( );
    }
    else if( !_current_record ) /// note: user is entering manual public key
    {
       elog( "!current record??\n" );
       /*
       if( _current_contact.known_since == QDateTime() )
       {
           _current_contact.known_since = QDateTime::currentDateTime();
       }
       */
    }
    _current_contact.privacy_setting = bts::addressbook::secret_contact;

    _address_book->storeContact( _current_contact );
    ui->info_stack->setCurrentWidget(ui->info_status);
    ui->chat_button->setEnabled(true);
    ui->mail_button->setEnabled(true);
    ui->info_button->setEnabled(true);
    ui->info_button->setChecked(true);
} FC_RETHROW_EXCEPTIONS( warn, "onSave" ) }

void ContactView::onCancel()
{
    ui->info_stack->setCurrentWidget(ui->info_status);
    ui->firstname->setText( _current_contact.first_name.c_str() );
    ui->lastname->setText( _current_contact.last_name.c_str() );
    ui->id_edit->setText( _current_contact.dac_id_string.c_str() );
    updateNameLabel();
}

void ContactView::onChat()
{
    ui->contact_pages->setCurrentWidget( ui->chat_page );
    //clear unread message count on display of chat window
    //DLNFIX maybe getMainWindow can be removed via some connect magic or similar observer notification?
    ContactGui* contact_gui = GetKeyhoteeWindow()->getContactGui(_current_contact.wallet_index);
    if (contact_gui)
        contact_gui->setUnreadMsgCount(0);
    ui->chat_input->setFocus();
}

void ContactView::onInfo()
{
    ui->info_stack->setCurrentWidget(ui->info_status);
    ui->contact_pages->setCurrentWidget( ui->info_page );
}

void ContactView::onMail()
{
    GetKeyhoteeWindow()->newMailMessageTo(_current_contact.wallet_index);
}

void ContactView::onDelete()
{
}


ContactView::~ContactView()
{
}

void ContactView::setContact( const Contact& current_contact,
                              ContactDisplay contact_display )
{ try {
    _current_contact = current_contact;
    bool has_null_public_key = _current_contact.public_key == fc::ecc::public_key_data();
    if ( has_null_public_key )
    {
        elog( "********* null public key!" );
        ui->save_button->setEnabled(false);
        ui->chat_button->setEnabled(false);
        ui->mail_button->setEnabled(false);
        ui->save_button->setEnabled(false);
        ui->info_button->setEnabled(false);
        ui->info_button->setChecked(true);
        onEdit();

        if (gMiningIsPossible)
           ui->id_status->setText( tr( "Please provide a valid ID or public key" ) );
        else
        {
           ui->id_status->setText( tr( "Public Key Only Mode" ) );
        }

        if( _current_contact.first_name == std::string() && _current_contact.last_name == std::string() )
        {
            ui->name_label->setText( tr( "New Contact" ) );
            ui->id_edit->setText( QString() );
            //keyhoteeIds don't function when mining is not possible
            if (!gMiningIsPossible)
               ui->id_edit->setEnabled(false);
        }
    }
    else
    {
        /// note: you cannot change the id of a contact once it has been
        /// set... you must create a new contact anytime their public key
        /// changes.
        ui->id_edit->setEnabled(false);
        ui->save_button->setEnabled(true);
        ui->chat_button->setEnabled(true);
        ui->mail_button->setEnabled(true);
        ui->info_button->setEnabled(true);

        if (contact_display == chat)
        {
            ui->chat_button->setChecked(true);
            onChat();
        }
        else
        {
            ui->info_button->setChecked(true);
            onInfo();

        }
            /** TODO... restore this kind of check
        if( _current_contact.bit_id_public_key != _current_contact.public_key  )
        {
            ui->id_status->setText( 
                    tr( "Warning! Keyhotee ID %1 no longer matches known public key!" ).arg(_current_contact.bit_id) );
        }
        */
    }

    ui->firstname->setText( _current_contact.first_name.c_str() );
    ui->lastname->setText( _current_contact.last_name.c_str() );
   // ui->email->setText( _current_contact.email_address );
   // ui->phone->setText( _current_contact.phone_number );
    /*
    auto vec   = fc::raw::pack( _current_contact.public_key );
    uint32_t check = fc::city_hash64( vec.data(), vec.size() );
    vec.resize( vec.size()+sizeof(check) );
    memcpy( &vec[vec.size()-sizeof(check)], (char*)&check, sizeof(check) );
    
    std::string base58_string = fc::to_base58( vec.data(), vec.size() );
    */
    ui->public_key_view->setText( std::string(public_key_address( _current_contact.public_key )).c_str() );//base58_string.c_str() );
    ui->id_edit->setText( _current_contact.dac_id_string.c_str() );
    ui->icon_view->setIcon( _current_contact.getIcon() );
} FC_RETHROW_EXCEPTIONS( warn, "" ) }

Contact ContactView::getContact()const
{
    return _current_contact;
}

void ContactView::firstNameChanged( const QString& /*name*/ )
{
    updateNameLabel();
}

void ContactView::updateNameLabel()
{
   auto full_name = ui->firstname->text() + " " + ui->lastname->text();
   if( full_name != " " )
   {
       ui->name_label->setText(full_name);
   }
   else if( ui->id_edit->text() != QString() )
   {
       ui->name_label->setText( ui->id_edit->text() );
   }
   else
   {
       ui->name_label->setText(tr( "New Contact" ));
   }
}

void ContactView::lastNameChanged( const QString& /*name*/ )
{
   updateNameLabel();
}

/*****************  Algorithm for handling keyhoteeId, keyhoteeeId status, and public key fields 
Notes:
If gMiningIsPossible,
  We can lookup a public key from a kehoteeId
  We can validate a public key is registered, 
    but we can't lookup the associated keyhoteeId, only a hash of the keyhoteeId

Some choices in Display Status for id not found on block chain: Available, Unable to find, Not registered

*** When creating new identity (this is for later implementation and some details may change):

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
void ContactView::keyhoteeIdEdited( const QString& id )
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
      ui->id_status->setText( tr( "Looking up id..." ) );
      fc::async( [=](){ 
          fc::usleep( fc::microseconds(500*1000) );
          if( fc::time_point::now() > (_last_validate + fc::microseconds(500*1000)) )
          {
             lookupId();
          }
      } );
   }
   updateNameLabel();
}

//implement real version and put in bitshares or fc (probably should be in fc)
bool is_valid_public_key(const std::string& public_key_string) 
{ 
   return public_key_address(public_key_string).is_valid();
} 

//implement real version and put in bitshares or fc (probably should be in fc)
bool is_registered_public_key(std::string public_key_string) 
{ 
   return false; //(public_key_string == "invictus");
} 

void ContactView::publicKeyEdited( const QString& public_key_string )
{
   ui->id_edit->clear(); //clear keyhotee id field
   if (gMiningIsPossible)
   {
      lookupPublicKey();
   }
   //check for validly hashed public key and enable/disable save button accordingly
   bool public_key_is_valid = is_valid_public_key(public_key_string.toStdString());
   if (public_key_is_valid)
   {
      ui->id_status->setText( tr("Public Key Only Mode: valid key") );
      ui->id_status->setStyleSheet("QLabel { color : green; }");
   }
   else
   {
      ui->id_status->setText( tr("Public Key Only Mode: not a valid key") );
      ui->id_status->setStyleSheet("QLabel { color : red; }");
   }
   ui->save_button->setEnabled(public_key_is_valid);
}

void ContactView::lookupId()
{
   try {
       auto current_id = ui->id_edit->text().toStdString();
       if( current_id.empty() )
       {
            ui->id_status->setText( QString() );
            ui->save_button->setEnabled(false);
            return;
       }
       _current_record = bts::application::instance()->lookup_name( current_id );
       if( _current_record )
       {
            ui->id_status->setStyleSheet("QLabel { color : green; }");
            ui->id_status->setText( tr( "Registered" ) );
            //DLNFIX replace with version that includes hash?
            std::string base58_string = bts::address(_current_record->pub_key);
            ui->public_key->setText( base58_string.c_str() );
            if( _address_book != nullptr )
               ui->save_button->setEnabled(true);
       }
       else
       {
            ui->id_status->setStyleSheet("QLabel { color : red; }");
            ui->id_status->setText( tr( "Unable to find ID" ) );
            ui->public_key->setText(QString());
            ui->save_button->setEnabled(false);
       }
   } 
   catch ( const fc::exception& e )
   {
      ui->id_status->setText( e.to_string().c_str() );
   }
}

void ContactView::lookupPublicKey()
{
   std::string public_key_string = ui->public_key->text().toStdString();
   //fc::ecc::public_key public_key;
   //bts::bitname::client::reverse_name_lookup( public_key );
   bool public_key_is_registered = is_registered_public_key(public_key_string);
   if (public_key_is_registered)
     ui->id_edit->setText("********"); //any better idea to indicate unknown but keyhoteeId?
   else
     ui->id_edit->setText(QString()); //clear keyhotee field if unregistered public key
}

void  ContactView::setAddressBook( AddressBookModel* addressbook )
{
    _address_book = addressbook;
}
AddressBookModel* ContactView::getAddressBook()const
{
    return _address_book;
}
