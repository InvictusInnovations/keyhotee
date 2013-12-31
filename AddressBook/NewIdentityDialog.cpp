#include "ui_NewIdentityDialog.h"
#include "public_key_address.hpp"
#include <bts/application.hpp>
#include "NewIdentityDialog.hpp"
#include <QPushButton>
#include <bts/addressbook/contact.hpp>
#include <fc/log/logger.hpp>

#include "KeyhoteeApplication.hpp"
#include "KeyhoteeMainWindow.hpp"
#include "AddressBookModel.hpp"


NewIdentityDialog::NewIdentityDialog( QWidget* parent_widget )
:QDialog(parent_widget),ui( new Ui::NewIdentityDialog() )
{
   ui->setupUi(this);
   ui->buttonBox->button( QDialogButtonBox::Save )->setEnabled(false);

   connect(ui->username, &QLineEdit::textChanged, this, &NewIdentityDialog::onUserNameChanged);
   connect(this, &QDialog::accepted, this, &NewIdentityDialog::onSave);
}

NewIdentityDialog::~NewIdentityDialog()
{
}

void NewIdentityDialog::onUserNameChanged( const QString& name )
{
   if( name != QString() )
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
          ui->status_label->setText( tr( "Status: You have already created this identity." ) );
      } 
      catch ( fc::key_not_found_exception& e )
      {
          ui->status_label->setText( tr( "Status: Unknown" ) );
          ui->buttonBox->button( QDialogButtonBox::Save )->setEnabled(true);
      }
   }
   else
   {
      ui->buttonBox->button( QDialogButtonBox::Save )->setEnabled(false);
   }
}

void NewIdentityDialog::onSave()
{
    //store new identity in profile
    auto app = bts::application::instance();
    auto profile = app->get_profile();
    auto trim_dac_id = fc::trim(ui->username->text().toUtf8().constData());
    bts::addressbook::wallet_identity ident;
    ident.first_name = fc::trim( ui->firstname->text().toUtf8().constData() );
    ident.last_name = fc::trim( ui->lastname->text().toUtf8().constData() );
    ident.mining_effort = ui->register_checkbox->isChecked();
    ident.wallet_ident = trim_dac_id;
    ident.set_dac_id( trim_dac_id );
    auto priv_key = profile->get_keychain().get_identity_key(trim_dac_id);
    ident.public_key = priv_key.get_public_key();
    profile->store_identity( ident );
    try 
    {
      app->mine_name(trim_dac_id,
                profile->get_keychain().get_identity_key(trim_dac_id).get_public_key(),
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
//    profile->get_addressbook()->store_contact(Contact(myself));
    TKeyhoteeApplication::getInstance()->getMainWindow()->getAddressBookModel()->storeContact( Contact(myself) );

    app->add_receive_key(priv_key);
}
