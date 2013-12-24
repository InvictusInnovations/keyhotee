#include "ui_NewIdentityDialog.h"
#include "NewIdentityDialog.hpp"

NewIdentityDialog::NewIdentityDialog( QWidget* parent_widget )
:QDialog(parent_widget),ui( new Ui::NewIdentityDialog() )
{
   ui->setupUi(this);
}

NewIdentityDialog::~NewIdentityDialog()
{
}
