#ifndef WALLETS_HPP
#define WALLETS_HPP

#include <QWidget>

namespace Ui {
class Wallets;
}

class QAuthenticator;
class QNetworkReply;

class Wallets : public QWidget
{
    Q_OBJECT

public:
  Wallets(QWidget* parent = nullptr);
  Wallets(QWidget* parent, const QString& url);
  ~Wallets();

private slots:
  void handleAuthenticationRequired(QNetworkReply*, QAuthenticator* authenticator);

private:
  void setupWebPage(QWidget* parent, const QString& url);

private:
    Ui::Wallets *ui;
};

#endif // WALLETS_HPP
