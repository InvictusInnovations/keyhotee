#ifndef MONEYATTACHEMENTWIDGET_HPP
#define MONEYATTACHEMENTWIDGET_HPP

#include <QWidget>

namespace Ui {
class MoneyAttachementWidget;
}

class TMoneyAttachementWidget : public QWidget
  {
  Q_OBJECT

  public:
    enum TCurrencyID
      {
      CURRENCY_BTC = 0,
      CURRENCY_LTC = 1,
      CURRENCY_BITUSD = 2
      };

    typedef std::pair<double, TCurrencyID> TValue;

    explicit TMoneyAttachementWidget(QWidget *parent = 0);
    virtual ~TMoneyAttachementWidget();

    TValue GetContents() const;

  private slots:
    /// Slot to update wallet balance label.
    void onCurrencyTypeChanged(int index);

  private:
    Ui::MoneyAttachementWidget *ui;
  };

#endif // MONEYATTACHEMENTWIDGET_HPP
