#include "mailfieldswidget.hpp"

#include "ui_mailfieldswidget.h"

#include <QAction>

MailFieldsWidget::MailFieldsWidget(QWidget& parent, QAction& actionSend) :
  QWidget(&parent),
  ui(new Ui::MailFieldsWidget),
  ActionSend(actionSend)
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

QString MailFieldsWidget::getSubject() const
  {
  return ui->subjectEdit->text();
  }

void MailFieldsWidget::showChildLayout(QLayout* layout, bool show, int preferredPosition)
  {
  ui->mailFieldsLayout->setEnabled(false);
  if(show)
    {
    int maxCnt = ui->mailFieldsLayout->count();
    if(preferredPosition >= maxCnt)
      preferredPosition = maxCnt;

    ui->mailFieldsLayout->insertLayout(preferredPosition, layout);
    }
  else
    {
    int itemIdx = 0;
    for(itemIdx = 0; itemIdx < ui->mailFieldsLayout->count(); ++itemIdx)
      {
      if(ui->mailFieldsLayout->itemAt(itemIdx) == layout)
        break;
      }

    ui->mailFieldsLayout->takeAt(itemIdx);
    }

  showLayoutWidgets(layout, show);

  ui->mailFieldsLayout->setEnabled(true);
  ui->mailFieldsLayout->invalidate();
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

void MailFieldsWidget::validateSendButtonState()
  {
  bool anyRecipient = ui->toEdit->text().isEmpty() == false;
  anyRecipient = anyRecipient || ui->ccEdit->text().isEmpty() == false;
  anyRecipient = anyRecipient || ui->bccEdit->text().isEmpty() == false;

  /// Make the send button enabled only if there is any recipient
  ui->sendButton->setEnabled(anyRecipient);
  }

void MailFieldsWidget::on_sendButton_clicked()
  {
  ActionSend.trigger();
  }

void MailFieldsWidget::on_toEdit_textChanged(const QString&)
  {
  validateSendButtonState();
  }

void MailFieldsWidget::on_bccEdit_textChanged(const QString&)
  {
  validateSendButtonState();
  }

void MailFieldsWidget::on_ccEdit_textChanged(const QString&)
  {
  validateSendButtonState();
  }

void MailFieldsWidget::onSubjectChanged(const QString &subject)
  {
  emit subjectChanged(subject);
  }

