#include "fileattachmentwidget.hpp"

#include "ui_fileattachmentwidget.h"

TFileAttachmentWidget::TFileAttachmentWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::TFileAttachmentWidget)
  {
  ui->setupUi(this);

  ui->addButton->setDefaultAction(ui->actionAdd);
  ui->delButton->setDefaultAction(ui->actionDel);
  ui->saveButton->setDefaultAction(ui->actionSave);

  /// Call it to validate buttons state at startup
  onAttachementTableSelectionChanged();
  }

TFileAttachmentWidget::~TFileAttachmentWidget()
  {
  delete ui;
  }

void TFileAttachmentWidget::ConfigureAttachmentTable()
  {

  }

void TFileAttachmentWidget::onAddTriggered()
  {
  }

void TFileAttachmentWidget::onDelTriggered()
  {
  }

void TFileAttachmentWidget::onSaveTriggered()
  {
  }

void TFileAttachmentWidget::onAttachementTableSelectionChanged()
  {
  auto selectedItems = ui->attachmentTable->selectedItems();
  bool anySelection = selectedItems.isEmpty();

  ui->actionDel->setEnabled(anySelection);
  ui->actionSave->setEnabled(anySelection);
  }

