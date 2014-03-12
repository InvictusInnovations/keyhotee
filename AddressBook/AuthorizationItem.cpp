#include "AuthorizationItem.hpp"


AuthorizationItem::~AuthorizationItem()
{
}

bool AuthorizationItem::isEqual(TPublicKey from_key) const
{
  return _from_key == from_key;
}

AuthorizationView* AuthorizationItem::getView() const
{
  if(childCount() > 0)
    return static_cast<AuthorizationItem*>(this->child(0))->getView();
  else
    return _view;
}
