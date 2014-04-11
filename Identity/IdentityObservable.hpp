#pragma once

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
  void notify ();

private:
  IdentityObservable() {};
  virtual ~IdentityObservable();

private:
  std::list<IIdentitiesUpdate*> _identObservers;
};


