#include "ContactView.hpp"
#include "ui_ContactView.h"

ContactView::ContactView( QWidget* parent )
:QWidget(parent),ui( new Ui::ContactView() )
{
   ui->setupUi(this);
}

ContactView::~ContactView()
{
}
