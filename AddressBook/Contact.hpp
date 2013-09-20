#pragma once
#include <QDateTime>
#include <QString>
#include <QIcon>
#include <fc/crypto/elliptic.hpp>

#include <bts/addressbook/contact.hpp>


/**
 *  Caches GUI-Specific data associated with the wallet.
 */
class Contact : public bts::addressbook::wallet_contact
{
   public:
      Contact(){}
      Contact( const bts::addressbook::wallet_contact& );

      QString        getLabel()const;
      const QIcon&   getIcon()const;
      void           setIcon( const QIcon& icon );
    
   private:
      /// cache the icon we want to use.
      QIcon   icon;
};

typedef std::shared_ptr<Contact> ContactPtr;
