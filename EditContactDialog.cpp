#include "EditContactDialog.hpp"
#include "ui_EditContactDialog.h"
#include <bts/application.hpp>
#include <fc/thread/thread.hpp>


EditContactDialog::EditContactDialog( QWidget* parent )
:QDialog(parent), ui( new Ui::EditContactDialog() ),_complete(false)
{
  ui->setupUi(this); 

  connect( ui->contact_id, &QLineEdit::textEdited,
           this, &EditContactDialog::validateId );
}

EditContactDialog::~EditContactDialog(){}

void EditContactDialog::validateId( const QString& id )
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
      //completeChanged();
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
}

void EditContactDialog::lookupId()
{
   try {
       auto cur_id = ui->contact_id->text().toStdString();
       if( cur_id == std::string() )
       {
            ui->id_status->setText( QString() );
            _complete = false;
            return;
       }
       auto opt_name_rec = bts::application::instance()->lookup_name( cur_id );
       if( opt_name_rec )
       {
            ui->id_status->setText( tr( "Valid ID" ) );
            _complete = true;
       //     completeChanged();
       }
       else
       {
            ui->id_status->setText( tr( "Unable to find ID" ) );
       }
   } 
   catch ( const fc::exception& e )
   {
      ui->id_status->setText( e.to_string().c_str() );
   }
}


