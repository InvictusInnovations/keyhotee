#pragma once

#include <bts/application.hpp>

/** Dedicated interface observed identities changes 
    (like add/delete identity)
*/
class IIdentitiesUpdate
{
public:
  typedef bts::addressbook::wallet_identity TIdentity;
  typedef std::vector<TIdentity>            TIdentities;

  /// Notify when the identities changed
  virtual void onIdentitiesChanged(const TIdentities& identities) = 0;
  virtual bool onIdentityDelIntent(const TIdentity&  identity) = 0;
  virtual bool onIdentityDelete(const TIdentity&  identity) = 0;

protected:
    virtual ~IIdentitiesUpdate() {}
};


