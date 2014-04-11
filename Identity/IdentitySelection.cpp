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
  QString currentText = "";
  if (!_identities.empty())
  {
    /// save curent selected identity name
    currentText = ui->identity_select->currentText();
  }

  _identities = identities;
  ui->identity_select->clear();
  int i = 0;
  for (const auto& v : _identities)
  {
    std::string entry = v.get_display_name();
    auto ipk = v.public_key;
    assert(ipk.valid());
    ui->identity_select->addItem(entry.c_str());
  }

  if (!_identities.empty())
  {
    if (currentText.isEmpty())
    {      
      ui->identity_select->setCurrentIndex(0);
    }
    else
    {
      int foundTextIdx = 0;
      /// select the saved identity name
      if ( (foundTextIdx = ui->identity_select->findText(currentText) ) == -1)
      {
        /// not found
        ui->identity_select->setCurrentIndex(0);
      }
      else
      {
        ui->identity_select->setCurrentIndex(foundTextIdx);
      }      
    }   
  }  

  bool show = (_identities.size() > 1);
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