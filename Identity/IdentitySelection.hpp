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
  const TIdentity* currentIdentity() const;

  /** Add widget associated with the IdentitySelection class.
      When identities size is <= 1 hide IdentitySelection widgets
      and also hide widgets related.
  */
  void addWidgetRelated(QWidget* widget);

protected:
/// IIdentitiesUpdate interface implementation:
  /// \see IIdentitiesUpdate interface description.
  virtual void onIdentitiesChanged(const TIdentities& identities) override;
  virtual bool onIdentityDelIntent(const TIdentity&  identity) override { return true; }
  virtual bool onIdentityDelete(const TIdentity&  identity) override { return true; }

private:
  Ui::IdentitySelection*  ui;
  TIdentities             _identities;
  QList<QWidget*>         _widgetsRelated;
};

#endif // IDENTITYSELECTION_H
