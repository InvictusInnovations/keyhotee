#include "wallets.hpp"
#include "ui_wallets.h"

#ifndef __STATIC_QT
#include <QtWebKitWidgets/QWebView>
#endif

Wallets::Wallets(QWidget* parent) :
QWidget(parent),
ui(new Ui::Wallets)
{
  ui->setupUi(this);

  ui->labelInfo->setText(tr("Please select wallet"));
  ui->labelInfo->setVisible(true);
}

Wallets::Wallets(QWidget* parent, const QString&  url) :
    QWidget(parent),
    ui(new Ui::Wallets)
{
  ui->setupUi(this);

#ifdef __STATIC_QT
  /// QWebView is not available
  ui->labelInfo->setText(tr("No support for Qt WebKit"));
  ui->labelInfo->setVisible(true);
#else
  /// Hide text: "No support for Qt WebKit"
  ui->labelInfo->setVisible(false);
  setupWebPage(parent, url);
#endif

}

Wallets::~Wallets()
{
  delete ui;
}

void Wallets::setupWebPage(QWidget* parent, const QString& url)
{
#ifndef __STATIC_QT
  QWebView *webView;
  webView = new QWebView(parent);
  webView->setObjectName(QStringLiteral("webView"));
  webView->setUrl(QUrl(QStringLiteral("about:blank")));
  ui->gridLayout->addWidget(webView, 0, 0, 1, 1);
    
  webView->settings()->setAttribute(QWebSettings::AutoLoadImages, true);
  webView->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);  
  webView->settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, false);
  webView->settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, false);
  webView->settings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
  webView->settings()->setAttribute(QWebSettings::PluginsEnabled, false);
  webView->settings()->setAttribute(QWebSettings::WebGLEnabled, false);    

  webView->load(QUrl(url));
#endif
}

/**
TODO
/// display some info about loading WebSite
void onLoadStarted();
/// hide info about loading WebSite
void onLoadFinished(bool);
*/