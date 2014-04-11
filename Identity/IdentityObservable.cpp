#include "IdentityObservable.hpp"
#include "IdentitiesUpdate.hpp"

IdentityObservable& IdentityObservable::getInstance()
{
  static IdentityObservable instance;
  return instance;
}

void IdentityObservable::addObserver (IIdentitiesUpdate* identityObserver)
{
  _identObservers.push_back(identityObserver);

  IIdentitiesUpdate::TIdentities idents;
  idents = bts::get_profile()->identities();
  /// Initialize TIdentities container
  identityObserver->onIdentitiesChanged(idents);
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
  IIdentitiesUpdate::TIdentities idents;
  idents = bts::get_profile()->identities();

  for (const auto& identityObserver : _identObservers)
  {
    identityObserver->onIdentitiesChanged(idents);
  }
}