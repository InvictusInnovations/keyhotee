#include "ContactView.hpp"
#include "ui_ContactView.h"

ContactView::ContactView( QWidget* parent )
:QWidget(parent),ui( new Ui::ContactView() )
{
   ui->setupUi(this);
   connect( ui->save_button, &QPushButton::clicked, this, &ContactView::onSave );
   connect( ui->cancel_button, &QPushButton::clicked, this, &ContactView::onCancel );
   connect( ui->edit_button, &QPushButton::clicked, this, &ContactView::onEdit );
}

void ContactView::onEdit()
{
    ui->info_stack->setCurrentIndex(1);
}
void ContactView::onSave()
{
    ui->info_stack->setCurrentIndex(0);
}

void ContactView::onCancel()
{
    ui->info_stack->setCurrentIndex(0);
}

ContactView::~ContactView()
{
}
