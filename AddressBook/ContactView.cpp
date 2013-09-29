#include "ContactView.hpp"
#include "ui_ContactView.h"
#include "AddressBookModel.hpp"

#include <KeyhoteeMainWindow.hpp>
#include <bts/application.hpp>

#include <fc/thread/thread.hpp>
#include <fc/log/logger.hpp>

#include <QWebFrame>

bool ContactView::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::KeyPress) 
  {
     QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
     switch(keyEvent->key()) 
     {
        case Qt::Key_Enter:
        case Qt::Key_Return:
           sendChatMessage();
           return true;
        default:
           break;
     }
  }
  return QObject::eventFilter(obj, event);
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
        auto pro = app->get_profile();
        auto  idents = pro->identities();
        bts::bitchat::private_text_message txt_msg( msg.toStdString() );
        if( idents.size() )
        {
           fc::ecc::private_key my_priv_key = pro->get_keychain().get_identity_key( idents[0].dac_id );
           app->send_text_message( txt_msg, _current_contact.public_key, my_priv_key );
           appendChatMessage( "me", msg );
        }

        ui->chat_input->setPlainText(QString());
    }
}
void ContactView::appendChatMessage( const QString& from, const QString& msg, const QDateTime& dateTime )
{ //DLNFIX2 improve formatting later
    wlog( "append... ${msg}", ("msg",msg.toStdString() ) );
    QString formatted_msg = dateTime.toString("hh:mm ap") + " "+ from + ": " + msg;
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
    QTextCursor cursor = ui->chat_conversation->textCursor();
    QString colorName = (from == "me") ? "grey" : "black";
    formatted_msg = QString("<font color=\"%1\">%2</font>").arg(colorName).arg(formatted_msg);
    ui->chat_conversation->insertHtml(formatted_msg);
    cursor.movePosition(QTextCursor::NextBlock);
    ui->chat_conversation->setTextCursor(cursor);
    #endif
}


ContactView::ContactView( QWidget* parent )
:QWidget(parent),ui( new Ui::ContactView() )
{
   _address_book = nullptr;
   _complete = false;
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
   connect( ui->id_edit, &QLineEdit::textChanged, this, &ContactView::keyhoteeIdChanged );

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

        ui->id_status->setText( tr( "Please provide a valid ID" ) );

        if( _current_contact.first_name == std::string() && _current_contact.last_name == std::string() )
        {
            ui->name_label->setText( tr( "New Contact" ) );
            ui->id_edit->setText( QString() );
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

void ContactView::keyhoteeIdChanged( const QString& id )
{
   /** TODO
    if( is_address( id ) )
   {
      _complete = true;
   }
   else
   */
   {
      _complete = false;
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

void ContactView::lookupId()
{
   try {
       auto cur_id = ui->id_edit->text().toStdString();
       if( cur_id == std::string() )
       {
            ui->id_status->setText( QString() );
            ui->save_button->setEnabled(false);
            _complete = false;
            return;
       }
       _current_record = bts::application::instance()->lookup_name( cur_id );
       if( _current_record )
       {
            ui->id_status->setText( tr( "Valid ID" ) );
            if( _address_book != nullptr )
               ui->save_button->setEnabled(true);
            _complete = true;
       }
       else
       {
            ui->id_status->setText( tr( "Unable to find ID" ) );
            ui->save_button->setEnabled(false);
       }
   } 
   catch ( const fc::exception& e )
   {
      ui->id_status->setText( e.to_string().c_str() );
   }
}
void  ContactView::setAddressBook( AddressBookModel* addressbook )
{
    _address_book = addressbook;
}
AddressBookModel* ContactView::getAddressBook()const
{
    return _address_book;
}
