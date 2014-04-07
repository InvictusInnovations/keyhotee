#pragma once

#include <list>

class IIdentitiesUpdate;

/** Register/Unregister identity observers.
    Notify observers when Identities changed.
*/
class IdentityObservable
{
public:
  /// Get instance of IdentityObservable (Singleton)
  static IdentityObservable& getInstance();

  void addObserver (IIdentitiesUpdate* identityObserver);
  void deleteObserver (IIdentitiesUpdate* identityObserver);
  void notify ();

protected:
  virtual ~IdentityObservable() {}

private:
  IdentityObservable() {};

private:
  static IdentityObservable     _instance;
  std::list<IIdentitiesUpdate*> _identObservers;
};


