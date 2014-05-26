#ifndef WALLETS_HPP
#define WALLETS_HPP

#include <QWidget>

namespace Ui {
class Wallets;
}

class QAuthenticator;
class QNetworkReply;
class QWebView;

class Wallets : public QWidget
{
    Q_OBJECT

public:
  Wallets(QWidget* parent = nullptr);
  Wallets(QWidget* parent, const QString& url);
  ~Wallets();

  /** QWebView initialization and load page from _url variable
      Is called when a user select the wallet on the treeitem
  */
  void loadPage();

private slots:
  void handleAuthenticationRequired(QNetworkReply*, QAuthenticator* authenticator);

private:
  /** QWebView initialization.
      Do not call setupWebPage in the Wallets constructor, because it crashes (only when program start with new profile) 
      as debug build on Windows.
      When QWebView calls QWebView::page() (line 211) method. Can't create QWebPage.
  */
  void setupWebPage();

private:
    Ui::Wallets *ui;
    QString     _url;
    QWebView*   _webView;
};

#endif // WALLETS_HPP
