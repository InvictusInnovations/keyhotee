#pragma once

#include <bts/application.hpp>

#include <QTreeWidget>

class AuthorizationView;

/**
 *  Navigation tree item representing incoming GUI entrypoint for an authorization request.
 */
class AuthorizationItem : public QTreeWidgetItem
{
public:
  typedef fc::ecc::public_key TPublicKey;

  AuthorizationItem(AuthorizationView* view, QTreeWidgetItem* parent, int type = 0)
    : QTreeWidgetItem(parent, type), _view(view) {}
  virtual ~AuthorizationItem();

  void setFromKey(TPublicKey from_key) {_from_key = from_key;}
  bool isEqual(TPublicKey from_key) const;

  AuthorizationView* getView() const;

/// Class attributes:
protected:
  AuthorizationView*  _view;
  TPublicKey          _from_key;
};

