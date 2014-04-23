#include "IdentitySelection.hpp"
#include "ui_IdentitySelection.h"

IdentitySelection::IdentitySelection(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IdentitySelection)
{
    ui->setupUi(this);
}

IdentitySelection::~IdentitySelection()
{
    delete ui;
}

void IdentitySelection::onIdentitiesChanged(const TIdentities& identities)
{
  TIdentity currentIdentity;
  int initialIndex = 0;
  /// save current selected identity
  int idx = ui->identity_select->currentIndex();
  if (idx != -1)
    currentIdentity = _identities[idx];

  _identities = identities;

  ui->identity_select->clear();
  int i = 0;
  /// Add identities to ComboBox
  for (const auto& v : identities)
  {
    std::string entry = v.get_display_name();
    auto ipk = v.public_key;
    assert(ipk.valid());
    ui->identity_select->addItem(entry.c_str());
    if (currentIdentity.public_key == v.public_key)
    {
      /// save current selected identity (don't clear selection)
      initialIndex = i;
    }
    ++i;
  }

  ui->identity_select->setCurrentIndex(initialIndex);

  bool show = (identities.size() > 1);
  this->setVisible(show);

  for (auto v : _widgetsRelated)
  {
    v->setVisible(show);
  }  
}


const IdentitySelection::TIdentity* IdentitySelection::currentIdentity() const
{
  if (_identities.size() == 1)
    return &_identities[0];

  /// get current selection from ComboBox
  int identityIdx = ui->identity_select->currentIndex();
  if (identityIdx != -1)
  {
    return &_identities[identityIdx];
  }   
  return nullptr;
}

void IdentitySelection::addWidgetRelated(QWidget* widget)
{
  _widgetsRelated.push_back(widget);
}