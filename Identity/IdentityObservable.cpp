#include "IdentityObservable.hpp"
#include "IdentitiesUpdate.hpp"

IdentityObservable IdentityObservable::_instance;

IdentityObservable& IdentityObservable::getInstance()
{
  return _instance;
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