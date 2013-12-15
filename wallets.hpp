#ifndef WALLETS_HPP
#define WALLETS_HPP

#include <QWidget>

namespace Ui {
  class Wallets;
  }

class Wallets : public QWidget
{
  Q_OBJECT

public:
  explicit Wallets(QWidget* parent = nullptr);
  ~Wallets();

private:
  Ui::Wallets *ui;
};

#endif // WALLETS_HPP
