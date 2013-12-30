#pragma once
#include <QDialog>

namespace Ui { class LoginDialog; }

class TKeyhoteeApplication;

class LoginDialog : public QDialog
{
public:
  LoginDialog(TKeyhoteeApplication& mainApp, QWidget* parent = nullptr);
  virtual ~LoginDialog();

private slots:
  void onLogin();
  void onQuit();
  void onNew();
  void shake();

  std::string            password;
private:
  /// Don't use unique_ptr here since it breaks QTCreator.
  Ui::LoginDialog*       ui;
  TKeyhoteeApplication&  _mainApp;
};
