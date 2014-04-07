#pragma once

#include <bts/application.hpp>

/** Dedicated interface observed identities changes 
    (like add/delete identity)
*/
class IIdentitiesUpdate
{
public:
    typedef std::vector<bts::addressbook::wallet_identity> TIdentities;

protected:
  virtual ~IIdentitiesUpdate() {}

  /// Notify when the identities changed
  virtual void onIdentitiesChanged(const TIdentities& identities) = 0;
};


