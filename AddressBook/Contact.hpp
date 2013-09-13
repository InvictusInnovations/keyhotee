#pragma once
#include <QDateTime>
#include <QString>
#include <QIcon>
#include <fc/crypto/elliptic.hpp>

#include <bts/addressbook/contact.hpp>

class Contact
{
   public:
      Contact():wallet_account_index(-1),privacy_setting(bts::addressbook::secret_contact){}

      QString getLabel()const
      {
          QString label = first_name + " " + last_name;
          if( label == " " )
          {
              if( bit_id != QString() )
              {
                 label = bit_id;
              }
              else
              {
                 label = fc::variant(public_key).as_string().substr(8).c_str();
              }
          }
          return label;
      }

      /// the account index used in our wallet.
      int32_t                    wallet_account_index;
      QIcon                      icon;
      QString                    first_name;
      QString                    last_name;
      QString                    company;
      QString                    bit_id;
      /// the public key currently associated with bit_id...
      fc::ecc::public_key_data   bit_id_public_key; 

      /// @note this is the key that identifies the contact,
      //  because this is how we first communicated with them,
      //  the bid_id and associated public key may change in which case
      //  the bit_id becomes invalid and we must alert the user that
      //  the bit_id no longer matches the key.
      fc::ecc::public_key_data          public_key;
      QDateTime                         known_since;
      bts::addressbook::privacy_level   privacy_setting;
      QString                           email_address;
      QString                           phone_number;
};
