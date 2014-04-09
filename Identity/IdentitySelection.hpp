#ifndef IDENTITYSELECTION_H
#define IDENTITYSELECTION_H

#include "IdentitiesUpdate.hpp"

#include <QWidget>

class TIdentities;

namespace Ui {
class IdentitySelection;
}

class IdentitySelection : public QWidget,
                          public IIdentitiesUpdate
{
  Q_OBJECT

public:
  typedef bts::addressbook::wallet_identity TIdentity;

  explicit IdentitySelection(QWidget *parent = 0);
  ~IdentitySelection();

  /** Returns current selected identity in the ComboBox.
      @note if there is no identities returns nullptr
  */
  const TIdentity* currentIdentity();

protected:
/// IIdentitiesUpdate interface implementation:
  /// \see IIdentitiesUpdate interface description.
  virtual void onIdentitiesChanged(const TIdentities& identities) override;

private:
  Ui::IdentitySelection*  ui;
  TIdentities             _identities;
};

#endif // IDENTITYSELECTION_H
