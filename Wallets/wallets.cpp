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
  
  webView->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
  webView->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
  webView->settings()->setAttribute(QWebSettings::JavaEnabled, true);
  webView->settings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
  webView->settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);
  webView->settings()->setAttribute(QWebSettings::WebGLEnabled, true);
  webView->settings()->setAttribute(QWebSettings::AutoLoadImages, true);

  webView->setHtml("", QUrl(url));
  webView->reload();
#endif
}

/**
TODO
/// display some info about loading WebSite
void onLoadStarted();
/// hide info about loading WebSite
void onLoadFinished(bool);
*/