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

  ConfigureContextMenu();
  ConfigureAttachmentTable();
  }

TFileAttachmentWidget::~TFileAttachmentWidget()
  {
  delete ui;
  }

void TFileAttachmentWidget::ConfigureContextMenu()
  {
  /** Register all actions to be displayed in ctx menu in a table widget (it should have configured
      ContextMenuPolicy to ActionsContextMenu).
  */
  ui->attachmentTable->addAction(ui->actionAdd);
  ui->attachmentTable->addAction(ui->actionDel);
  ui->attachmentTable->addAction(ui->actionSave);
  ui->attachmentTable->addAction(ui->actionRename);
  QAction* sep = new QAction(this);
  sep->setSeparator(true);
  ui->attachmentTable->addAction(sep);
  ui->attachmentTable->addAction(ui->actionSelectAll);
  }

void TFileAttachmentWidget::ConfigureAttachmentTable()
  {
  QSize tableSize = ui->attachmentTable->size();
  ui->attachmentTable->setColumnWidth(0, 2/3*tableSize.width());
  ui->attachmentTable->setColumnWidth(1, 1/3*tableSize.width());

  UpdateColumnHeaders(0, 0);
  }

void TFileAttachmentWidget::UpdateColumnHeaders(unsigned int count, unsigned long long totalSize)
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

