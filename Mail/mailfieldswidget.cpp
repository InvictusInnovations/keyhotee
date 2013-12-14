#include "mailfieldswidget.hpp"

#include "maileditorwindow.hpp"

#include "ui_mailfieldswidget.h"

#include <QMenu>

MailFieldsWidget::MailFieldsWidget(MailEditorMainWindow& parent) :
    QWidget(&parent),
    MainEditor(parent),
    ui(new Ui::MailFieldsWidget)
  {
  ui->setupUi(this);
  }

MailFieldsWidget::~MailFieldsWidget()
  {
  delete ui;
  }

void MailFieldsWidget::showFromControls(bool show)
  {
  showChildLayout(ui->fromLayout, show, 0);
  }

void MailFieldsWidget::showCcControls(bool show)
  {
  showChildLayout(ui->ccLayout, show, 2);
  }

void MailFieldsWidget::showBccControls(bool show)
  {
  showChildLayout(ui->bccLayout, show, 3);
  }

void MailFieldsWidget::showChildLayout(QLayout* layout, bool show, int preferredPosition)
  {
  ui->gridLayout->setEnabled(false);
  if(show)
    {
    int maxCnt = ui->gridLayout->rowCount();
    if(preferredPosition >= maxCnt)
       preferredPosition = maxCnt - 1;

    ui->gridLayout->addLayout(layout, preferredPosition, 1, 1, 1);
    }
  else
    {
    int itemIdx = 0;
    for(itemIdx = 0; itemIdx < ui->gridLayout->count(); ++itemIdx)
      {
      if(ui->gridLayout->itemAt(itemIdx) == layout)
        break;
      }

    ui->gridLayout->takeAt(itemIdx);
    }

  showLayoutWidgets(layout, show);

  ui->gridLayout->setEnabled(true);
  ui->gridLayout->invalidate();

  }

void MailFieldsWidget::showLayoutWidgets(QLayout* layout, bool show)
  {
  for(int i = 0; i < layout->count(); ++i)
    {
    QLayoutItem* li = layout->itemAt(i);
    QWidget* w = li->widget();
    w->setVisible(show);
    w->setEnabled(show);
    }
  }

void MailFieldsWidget::on_sendButton_clicked()
{

}
