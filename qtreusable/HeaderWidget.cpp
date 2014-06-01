#include "HeaderWidget.hpp"
#include "ui_HeaderWidget.h"

HeaderWidget::HeaderWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HeaderWidget)
{
    ui->setupUi(this);
}

HeaderWidget::~HeaderWidget()
{
    delete ui;
}

void HeaderWidget::initial(QString title)
{
  ui->title->setText(title);
}

void HeaderWidget::onHeaderChanged(QString newTitle)
{
  ui->title->setText(newTitle);
}
