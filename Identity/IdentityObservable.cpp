#include "IdentityObservable.hpp"

#include <bts/addressbook/addressbook.hpp>
#include <fc/log/logger.hpp>

IdentityObservable::~IdentityObservable()
{
  assert(_identObservers.empty());
}

IdentityObservable& IdentityObservable::getInstance()
{
  static IdentityObservable instance;
  return instance;
}

void IdentityObservable::addObserver (IIdentitiesUpdate* identityObserver)
{
  _identObservers.push_back(identityObserver);

  if (_identities.empty())
    reloadIdentities();

  /// Initialize TIdentities container
  identityObserver->onIdentitiesChanged(_identities);
}

void IdentityObservable::deleteObserver (IIdentitiesUpdate* identityObserver)
{
  _identObservers.remove_if( [=](IIdentitiesUpdate* deleteObserver)
                             { 
                               /// find and remove observer
                               return deleteObserver == identityObserver;
                             }
                           );
}

void IdentityObservable::notify ()
{
  reloadIdentities();
  notifyObservers();
}

void IdentityObservable::notify(const bts::addressbook::wallet_contact& contact)
{
  for (auto& identity : _identities)
  {
    if (identity.public_key == contact.public_key)
    {
      /// Update alias in the identity
      identity.first_name = contact.first_name;
      identity.last_name = contact.last_name;

      notifyObservers();
      return;
    }
  }
}

void IdentityObservable::notifyObservers()
{
  for (const auto& identityObserver : _identObservers)
  {
    identityObserver->onIdentitiesChanged(_identities);
  }
}

void IdentityObservable::reloadIdentities()
{
  bts::profile_ptr profile = bts::get_profile();
  _identities = profile->identities();

  bts::addressbook::addressbook_ptr addressbook = profile->get_addressbook();

  for (auto& v : _identities)
  {
    try
    {
      /// Find contact containing identity public_key
      auto findContact = addressbook->get_contact_by_public_key(v.public_key);
      if (findContact)
      {
        /// copy contact alias to identity
        v.first_name = findContact->first_name;
        v.last_name = findContact->last_name;
      }
    }
    catch (const fc::exception& e)
    {
      elog("${e}", ("e", e.to_detail_string()));
    }
  } 
}

bool IdentityObservable::notifyDelIntent(const IIdentitiesUpdate::TIdentity& identity)
{
  bool b = true;
  for(const auto& identityObserver : _identObservers)
  {
    b &= identityObserver->onIdentityDelIntent(identity);
  }
  return b;
}

bool IdentityObservable::notifyDelete(const IIdentitiesUpdate::TIdentity& identity)
{
  bool b = true;
  for(const auto& identityObserver : _identObservers)
  {
    b &= identityObserver->onIdentityDelete(identity);
  }
  return b;
}
