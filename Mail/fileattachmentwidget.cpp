#include "fileattachmentwidget.hpp"

#include "ui_fileattachmentwidget.h"

TFileAttachmentWidget::TFileAttachmentWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::TFileAttachmentWidget)
  {
  ui->setupUi(this);

  }

TFileAttachmentWidget::~TFileAttachmentWidget()
  {
  delete ui;
  }

