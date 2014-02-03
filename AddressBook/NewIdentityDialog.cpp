#include "ui_NewIdentityDialog.h"
#include "public_key_address.hpp"
#include <bts/application.hpp>
#include "NewIdentityDialog.hpp"
#include <QPushButton>
#include <bts/addressbook/contact.hpp>
#include <fc/log/logger.hpp>
#include <fc/thread/thread.hpp>

#include "KeyhoteeApplication.hpp"
#include "KeyhoteeMainWindow.hpp"
#include "AddressBookModel.hpp"

#include <iostream>
#include <bts/application.hpp>
#include <fc/filesystem.hpp>
#include <fc/io/json.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/fstream.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/rpc/json_connection.hpp>
#include <fc/interprocess/signals.hpp>
#include <iostream>


NewIdentityDialog::NewIdentityDialog( QWidget* parent_widget )
:QDialog(parent_widget),ui( new Ui::NewIdentityDialog() )
{
   ui->setupUi(this);
   ui->buttonBox->button( QDialogButtonBox::Save )->setEnabled(false);

   connect(ui->username, &QLineEdit::textChanged, this, &NewIdentityDialog::onUserNameChanged);
   connect(ui->founder_code, &QLineEdit::textChanged, this, &NewIdentityDialog::onKey);
   connect(this, &QDialog::accepted, this, &NewIdentityDialog::onSave);
}

NewIdentityDialog::~NewIdentityDialog()
{
}

void NewIdentityDialog::onUserNameChanged( const QString& name )
{
   if( name.trimmed() != QString() )
   {
      auto pro = bts::application::instance()->get_profile();
      auto keys = pro->get_keychain();

      auto trim_name = fc::trim(name.toUtf8().constData());
      auto ident_key = keys.get_identity_key( trim_name );
      auto key_addr  = /*bts::*/public_key_address( ident_key.get_public_key() );
      ui->publickey->setText( std::string(key_addr).c_str() );

      // make sure the key is unique..
      try {
          bts::addressbook::wallet_identity cur_ident = pro->get_identity( trim_name );
          ui->buttonBox->button( QDialogButtonBox::Save )->setEnabled(false);
          ui->status_label->setStyleSheet("QLabel { color : red; }");
          ui->status_label->setText( tr( "Status: You have already created this identity." ) );
      } 
      catch ( fc::key_not_found_exception& )
      {
          fc::optional<bts::bitname::name_record> current_record = bts::application::instance()->lookup_name(trim_name);
          if(current_record)
          {
             ui->status_label->setStyleSheet("QLabel { color : red; }");
             ui->status_label->setText(tr("Status: This Keyhotee ID was already registered by someone else."));
             ui->buttonBox->button( QDialogButtonBox::Save )->setEnabled(false);
          }
          else
          {
             ui->status_label->setStyleSheet("QLabel { color : black; }");
             ui->status_label->setText( tr( "Status: Unknown" ) );
             ui->buttonBox->button( QDialogButtonBox::Save )->setEnabled(true);
          }
      }
   }
   else
   {
      ui->buttonBox->button( QDialogButtonBox::Save )->setEnabled(false);
   }
}

fc::string register_key_on_server(const QString& keyhotee_id, const QString& key)
{
  fc::string trimmed_keyhotee_id = keyhotee_id.trimmed().toStdString();
  fc::string trimmed_key = key.trimmed().toStdString();
  auto pro = bts::application::instance()->get_profile();
  auto keys = pro->get_keychain();
  auto ident_key       = keys.get_identity_key( trimmed_keyhotee_id );
  std::string key_addr = public_key_address( ident_key.get_public_key() );

  fc::tcp_socket_ptr sock = std::make_shared<fc::tcp_socket>();
  sock->connect_to( fc::ip::endpoint( fc::ip::address("162.243.67.4"), 3879 ) );
  fc::buffered_istream_ptr isock = std::make_shared<fc::buffered_istream>(sock);
  fc::buffered_ostream_ptr osock = std::make_shared<fc::buffered_ostream>(sock);
  fc::rpc::json_connection_ptr con = std::make_shared<fc::rpc::json_connection>( isock, osock );
  con->exec();

  auto result = con->async_call( "register_key", trimmed_keyhotee_id, trimmed_key, key_addr ).wait();
  auto points = result.get_object()["points"].as_string();
  auto pub    = result.get_object()["pub_key"].as_string();
  return points;
}

void display_founder_key_status(const QString& keyhotee_id, const QString& key, QLabel* status_label)
{
   try {
      auto points = register_key_on_server(keyhotee_id, key);
      fc::usleep(fc::seconds(1));
      status_label->setText( ("Registered with " + points + " points").c_str() );
   } 
   catch ( fc::eof_exception&  )
   {
   ilog("eof exception");
   }
   catch ( fc::exception& e )
   {
      fc::string msg = "Invalid KehoteeID/FounderKey Pair: " + e.to_string();
      elog("${msg}",("msg",msg));
      status_label->setText( "Invalid ID/FounderKey Pair" );
   }
   catch ( ... )
   {
      elog("unknown error while displaying founder key status");
      status_label->setText( "Error Registering Founder ID" );
   }
}

void NewIdentityDialog::onKey( const QString& key )
{
  display_founder_key_status(ui->username->text(), key, ui->status_label);
}

//#ifndef _DEBUG
/// defined in CmakeLists.txt
//#define ALPHA_RELEASE
 //Q&D hack: remove all references to this for real release
//#endif

void NewIdentityDialog::onSave()
{
    //store new identity in profile
    auto app = bts::application::instance();
    auto profile = app->get_profile();
    auto trimmed_dac_id = ui->username->text().trimmed();
    fc::string dac_id = trimmed_dac_id.toStdString();

    // make sure the key is unique..
    try {
        bts::addressbook::wallet_identity cur_ident = profile->get_identity( dac_id );
        ui->buttonBox->button( QDialogButtonBox::Save )->setEnabled(false);
        ui->status_label->setStyleSheet("QLabel { color : red; }");
        ui->publickey->setText( "" );
        ui->status_label->setText( tr( "Status: You have already created this identity." ) );
        return;
    }
    catch ( fc::key_not_found_exception& )
    {
        fc::optional<bts::bitname::name_record> current_record = bts::application::instance()->lookup_name(dac_id);
        if(current_record)
        {
           ui->status_label->setStyleSheet("QLabel { color : red; }");
           ui->status_label->setText(tr("Status: This Keyhotee ID was already registered by someone else."));
           ui->publickey->setText( "" );
           ui->buttonBox->button( QDialogButtonBox::Save )->setEnabled(false);
           return;
        }
    }


    bts::addressbook::wallet_identity ident;
    ident.first_name = fc::trim( ui->firstname->text().toUtf8().constData() );
    ident.last_name = fc::trim( ui->lastname->text().toUtf8().constData() );
    ident.mining_effort = ui->register_checkbox->isChecked();
    ident.wallet_ident = dac_id;
    ident.set_dac_id( dac_id );
    auto priv_key = profile->get_keychain().get_identity_key(dac_id);
    ident.public_key = priv_key.get_public_key();
    profile->store_identity( ident );
    try 
    {
      app->mine_name(dac_id,
                profile->get_keychain().get_identity_key(dac_id).get_public_key(),
                ident.mining_effort);
    }
    catch ( const fc::exception& e )
    {
      wlog( "${e}", ("e",e.to_detail_string()) );
    }

    //store contact for new identity
    bts::addressbook::wallet_contact myself;
    //copy common fields from new identity into contact
    static_cast<bts::addressbook::contact&>(myself) = ident;
    //fill in remaining fields for contact
    myself.first_name = ident.first_name;
    myself.last_name = ident.last_name;
#ifdef ALPHA_RELEASE
   //just store the founder code here so we can access it over in ContactView temporarily for alpha release
    myself.notes = ui->founder_code->text().toStdString();
#endif
    TKeyhoteeApplication::getInstance()->getMainWindow()->getAddressBookModel()->storeContact( Contact(myself) );

    app->add_receive_key(priv_key);
    emit identityadded();

}
