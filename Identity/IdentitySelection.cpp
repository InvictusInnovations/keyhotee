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
    ui->identity_select->addItem(v.get_display_name().c_str());
    ++i;
  }

  if (identities.empty() == false)
    ui->identity_select->setCurrentIndex(0);

  bool show = (identities.size() > 1);
  ui->identity_select->setVisible(show);
  ui->identity_label->setVisible(show);
}


std::string IdentitySelection::getCurrentIdentity()
{
  if (_identities.size () == 1)
    return _identities[0].dac_id_string;

  /// get current selection from ComboBox
  int identityIdx = ui->identity_select->currentIndex();
  if (identityIdx != -1)
  {
    return _identities[identityIdx].dac_id_string;
  }   
  return "";
}