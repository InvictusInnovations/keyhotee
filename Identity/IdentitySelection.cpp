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
  _identities = identities;

  ui->identity_select->clear();
  int i = 0;
  for (const auto& v : identities)
  {
    std::string entry = v.get_display_name();
    auto ipk = v.public_key;
    assert(ipk.valid());
    ui->identity_select->addItem(entry.c_str());
  }

  if (identities.empty() == false)
    ui->identity_select->setCurrentIndex(0);

  bool show = (identities.size() > 1);
  ui->identity_select->setVisible(show);
  ui->identity_label->setVisible(show);
}


const IdentitySelection::TIdentity* IdentitySelection::currentIdentity()
{
  if (_identities.size () == 1)
    return &_identities[0];

  /// get current selection from ComboBox
  int identityIdx = ui->identity_select->currentIndex();
  if (identityIdx != -1)
  {
    return &_identities[identityIdx];
  }   
  return nullptr;
}