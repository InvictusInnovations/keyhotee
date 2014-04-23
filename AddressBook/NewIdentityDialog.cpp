#include "NewIdentityDialog.hpp"
#include "ui_NewIdentityDialog.h"

#include "AddressBookModel.hpp"
#include "Contact.hpp"
#include "KeyhoteeApplication.hpp"
#include "KeyhoteeMainWindow.hpp"
#include "public_key_address.hpp"

#include "Identity/IdentityObservable.hpp"

#include <QPushButton>

#include <bts/application.hpp>
#include <bts/addressbook/contact.hpp>

#include <fc/filesystem.hpp>
#include <fc/interprocess/signals.hpp>
#include <fc/io/json.hpp>
#include <fc/io/fstream.hpp>
#include <fc/log/logger.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/rpc/json_connection.hpp>
#include <fc/thread/thread.hpp>

#include <iostream>


NewIdentityDialog::NewIdentityDialog( QWidget* parent_widget )
:QDialog(parent_widget),ui( new Ui::NewIdentityDialog() )
{
   ui->setupUi(this);

   const int WEEK = 7;
   const int MONTH = 4 * WEEK;
   const int YEAR = 12 * MONTH;
   const int ALL = -1;
   ui->downloadHistory->addItem(tr("No History"), 0);
   ui->downloadHistory->addItem(tr("1 week"), WEEK);
   ui->downloadHistory->addItem(tr("1 month"), MONTH);
   ui->downloadHistory->addItem(tr("3 months"), 3*MONTH);
   ui->downloadHistory->addItem(tr("6 months"), 6*MONTH);
   ui->downloadHistory->addItem(tr("1 year"), YEAR);
   ui->downloadHistory->addItem(tr("2 years"), 2*YEAR);
   ui->downloadHistory->addItem(tr("3 years"), 3*YEAR);
   ui->downloadHistory->addItem(tr("5 years"), 5*YEAR);
   ui->downloadHistory->addItem(tr("all"), ALL);

   ui->buttonBox->button( QDialogButtonBox::Save )->setEnabled(false);

   connect(ui->username, &QLineEdit::textChanged, this, &NewIdentityDialog::onUserNameChanged);
   connect(ui->founder_code, &QLineEdit::textChanged, this, &NewIdentityDialog::onKey);
   connect(this, &QDialog::accepted, this, &NewIdentityDialog::onSave);
}

NewIdentityDialog::~NewIdentityDialog()
{
  delete ui;
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
          if (cur_ident.dac_id_string == trim_name)
              ui->status_label->setText( tr( "Status: You have already created this identity." ) );
          else
              ui->status_label->setText( tr( "Status: You have already created a similar id." ) );
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
      catch ( fc::assert_exception& )
      {
          ui->status_label->setStyleSheet("QLabel { color : red; }");
          ui->status_label->setText(tr("Status: Invalid ID. This ID cannot be registered."));
          ui->buttonBox->button( QDialogButtonBox::Save )->setEnabled(false);
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

void display_founder_key_status(const QString& keyhotee_id, const QString& founder_code, QLabel* status_label)
{
   try {
      auto points = register_key_on_server(keyhotee_id, founder_code);
      fc::usleep(fc::seconds(1));
      status_label->setText( ("Registered with " + points + " points").c_str() );
   } 
   catch ( fc::eof_exception&  )
   {
   ilog("eof exception");
   }
   catch ( fc::exception& e )
   {
      fc::string msg = "Founder Key Status exception: " + e.to_string();
      elog("${msg}",("msg",msg));
      if (founder_code.size())
      {
        status_label->setText( "Invalid ID/FounderKey Pair" );
      }
      else
      {
        status_label->setText( "Unregistered or failed to reach server" );
      }
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
      if (cur_ident.dac_id_string == dac_id)
          ui->status_label->setText( tr( "Status: You have already created this identity." ) );
      else
          ui->status_label->setText( tr( "Status: You have already created a similar id." ) );
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
  ident.mining_effort = ui->register_checkbox->isChecked() ? 25.0 : 0.0;
  ident.wallet_ident = dac_id;
  ident.set_dac_id( dac_id );
  auto priv_key = profile->get_keychain().get_identity_key(dac_id);
  ident.public_key = priv_key.get_public_key();
  profile->store_identity( ident );
  /// notify identity observers
  IdentityObservable::getInstance().notify();
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

	//re-connect to mail server to send an updated sync_time (get past mail for identities) if history should be downloaded
	int days_in_past_to_fetch = ui->downloadHistory->currentData().toInt();
  if (days_in_past_to_fetch != 0)
  {
    fc::time_point sync_time;
    if (days_in_past_to_fetch != -1)
      sync_time = fc::time_point::now() - fc::days(days_in_past_to_fetch);
    //if requested sync time further in the past than last sync time, change last_sync_time
    if (sync_time < profile->get_last_sync_time())
      profile->set_last_sync_time(sync_time);
    app->connect_to_network();
  }
  emit identityadded();

}
