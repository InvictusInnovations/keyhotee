#include "moneyattachementwidget.hpp"
#include "ui_moneyattachementwidget.h"

#include <QIcon>

#include <assert.h>

TMoneyAttachementWidget::TMoneyAttachementWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::MoneyAttachementWidget)
  {
  ui->setupUi(this);

  /** TODO Probably list of available currencies should be taken from defined lists of wallets.
  */
  ui->currencyComboBox->insertItem(CURRENCY_BTC, QIcon::fromTheme("currency-bitcoin",
    QIcon(":/images/bitcoin.png") ), QString("BTC"));
  ui->currencyComboBox->insertItem(CURRENCY_LTC, QIcon::fromTheme("currency-litecoin",
    QIcon(":/images/litecoin128.png") ), QString("LTC"));
  ui->currencyComboBox->insertItem(CURRENCY_BITUSD, QIcon::fromTheme("currency-bitusd",
    QIcon(":/images/bitusd.png") ), QString("BitUSD"));
  }

TMoneyAttachementWidget::~TMoneyAttachementWidget()
  {
  delete ui;
  }

TMoneyAttachementWidget::TValue TMoneyAttachementWidget::GetContents() const
  {
  TValue value;
  value.first = ui->amountBox->value();
  value.second = static_cast<TCurrencyID>(ui->currencyComboBox->currentIndex());

  return value;
  }

void TMoneyAttachementWidget::onCurrencyTypeChanged(int index)
  {
  switch (index)
    {
  case CURRENCY_BTC:
    ui->balanceLabel->setText("Balance: 17.76 BTC");
    break;
  case CURRENCY_LTC:
    ui->balanceLabel->setText("Balance: 0.00 LTC");
    break;
  case CURRENCY_BITUSD:
    ui->balanceLabel->setText("Balance: 0.00 BitUSD");
    break;
  default:
    assert(false);
    }
  }

