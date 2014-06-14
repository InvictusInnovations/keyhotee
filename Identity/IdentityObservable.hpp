#pragma once

#include "IdentitiesUpdate.hpp"
#include <list>

class IIdentitiesUpdate;

/** This singleton class manages identity observers.
    Notify observers when identities changed.
 */ 
class IdentityObservable
{
public:
  /// Get instance of IdentityObservable
  static IdentityObservable& getInstance();

/// Observer methods
  void addObserver (IIdentitiesUpdate* identityObserver);
  void deleteObserver (IIdentitiesUpdate* identityObserver);
  /// Called when identity will be added/deleted
  void notify();
  /** Called when ownership contact alias will be changed.
      The same as notify() but more efficient. Not reload all
      identities but just change alias in the identity
  */
  void notify(const bts::addressbook::wallet_contact& contact);

  bool notifyDelIntent(const IIdentitiesUpdate::TIdentity& identity);
  bool notifyDelete(const IIdentitiesUpdate::TIdentity& identity);

private:
  IdentityObservable() {};
  virtual ~IdentityObservable();
  void reloadIdentities();
  void notifyObservers();

private:
  std::list<IIdentitiesUpdate*> _identObservers;
  IIdentitiesUpdate::TIdentities _identities;
};


