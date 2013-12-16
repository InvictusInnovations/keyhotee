#pragma once
#include <QDateTime>
#include <QString>
#include <QIcon>
#include <fc/crypto/elliptic.hpp>

#include <bts/addressbook/contact.hpp>


/**
 *  Caches GUI-Specific data associated with the wallet_contact.
 */
class Contact : public bts::addressbook::wallet_contact
{
   public:
      Contact(){}
      explicit Contact( const bts::addressbook::wallet_contact& );
      /// Returns true if the identity associated to given contact is owned by current profile's identity.
      bool           isOwn() const;
      QString        getLabel()const;
      const QIcon&   getIcon()const;
      void           setIcon( const QIcon& icon );
      int            getAge()const;
      int            getRepute()const;
    
   private:
      /// cache the icon we want to use.
      QIcon   icon;
};

typedef std::shared_ptr<Contact> ContactPtr;
