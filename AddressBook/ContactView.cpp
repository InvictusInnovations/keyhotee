#include "ContactView.hpp"
#include "ui_ContactView.h"

#include <bts/application.hpp>
#include <fc/thread/thread.hpp>

#include "AddressBookModel.hpp"
ContactView::ContactView( QWidget* parent )
:QWidget(parent),ui( new Ui::ContactView() )
{
   _address_book = nullptr;
   _complete = false;
   ui->setupUi(this);
   connect( ui->save_button, &QPushButton::clicked, this, &ContactView::onSave );
   connect( ui->cancel_button, &QPushButton::clicked, this, &ContactView::onCancel );
   connect( ui->edit_button, &QPushButton::clicked, this, &ContactView::onEdit );
   connect( ui->chat_button, &QAbstractButton::clicked, this, &ContactView::onChat );
   connect( ui->info_button, &QAbstractButton::clicked, this, &ContactView::onInfo );

   connect( ui->firstname, &QLineEdit::textChanged, this, &ContactView::firstNameChanged );
   connect( ui->lastname, &QLineEdit::textChanged, this, &ContactView::lastNameChanged );
   connect( ui->id_edit, &QLineEdit::textChanged, this, &ContactView::keyhoteeIdChanged );

   setContact( Contact() );
}

void ContactView::onEdit()
{
    ui->info_stack->setCurrentWidget(ui->info_edit);
}
void ContactView::onSave()
{
    _current_contact.first_name = ui->firstname->text();
    _current_contact.last_name = ui->lastname->text();
    _address_book->storeContact( _current_contact );
    ui->info_stack->setCurrentWidget(ui->info_status);
    ui->chat_button->setEnabled(true);
    ui->mail_button->setEnabled(true);
    ui->info_button->setEnabled(true);
    ui->info_button->setChecked(true);
}

void ContactView::onCancel()
{
    ui->info_stack->setCurrentWidget(ui->info_status);
    ui->firstname->setText( _current_contact.first_name );
    ui->lastname->setText( _current_contact.last_name );
    ui->id_edit->setText( _current_contact.bit_id );
    updateNameLabel();
}

void ContactView::onChat()
{
    ui->contact_pages->setCurrentWidget( ui->chat_page );
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
void    ContactView::setContact( const Contact& current_contact )
{
    _current_contact = current_contact;
    if( _current_contact.public_key == fc::ecc::public_key_data() )
    {
        ui->save_button->setEnabled(false);
        ui->chat_button->setEnabled(false);
        ui->mail_button->setEnabled(false);
        ui->save_button->setEnabled(false);
        ui->id_status->setText( tr( "Please provide a valid ID" ) );

        if( _current_contact.first_name == QString() &&
            _current_contact.last_name == QString() )
        {
            ui->name_label->setText( tr( "New Contact" ) );
        }
    }
    else
    {
        /// note: you cannot change the id of a contact once it has been
        /// set... you must create a new contact anytime their public key
        /// changes.
        ui->id_edit->setEnabled(false);
        ui->save_button->setEnabled(true);

        if( _current_contact.bit_id_public_key != _current_contact.public_key  )
        {
            ui->id_status->setText( 
                 tr( "Warning! Keyhotee ID %1 no longer matches known public key!" ).arg(_current_contact.bit_id) );
        }
    }

    ui->firstname->setText( _current_contact.first_name );
    ui->lastname->setText( _current_contact.last_name );
    ui->email->setText( _current_contact.email_address );
    ui->phone->setText( _current_contact.phone_number );

    if( !_current_contact.icon.isNull() )
    {
      ui->icon_view->setIcon( _current_contact.icon );
    }
    else
    {
      QIcon icon;
      icon.addFile(QStringLiteral(":/images/user.png"), QSize(), QIcon::Normal, QIcon::Off);
      ui->icon_view->setIcon(icon);
    }

}

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
