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
  /// for root wallet item
  Wallets(QWidget* parent = nullptr);
  /// for wallet item
  Wallets(QWidget* parent, const QString& url, const uint port);
  ~Wallets();

  /** QWebView initialization and load page from _url variable
      Is called when a user select the wallet on the treeitem
  */
  void loadPage();
  void onWaitingForServer();
  void setAuthentication(QString username, QString password);

private slots:
  /// QWebPage load started
  void onLoadStarted();
  void onLoadProgress(int progress);
  /** QWebPage load finished
      @param loadOK - is true when QWebPage load was successful
  */
  void onLoadFinished(bool loadOK);
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
    uint        _port;
    QWebView*   _webView;
    QString     _username;
    QString     _password;
};

#endif // WALLETS_HPP
