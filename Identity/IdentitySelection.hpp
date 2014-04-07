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
  explicit IdentitySelection(QWidget *parent = 0);
  ~IdentitySelection();

  /// Returns keyhoteeId from current selected identity in the ComboBox.
  std::string getCurrentIdentity();

protected:
/// IIdentitiesUpdate interface implementation:
  /// \see IIdentitiesUpdate interface description.
  virtual void onIdentitiesChanged(const TIdentities& identities) override;

private:
  Ui::IdentitySelection*  ui;
  TIdentities             _identities;
};

#endif // IDENTITYSELECTION_H
