#pragma once

#include <QDialog>

namespace Ui {
class LoginInputBox;
}

/// Class returns username and password entered by the user
class LoginInputBox : public QDialog
{
    Q_OBJECT

public:
  LoginInputBox(QWidget *parent, const QString& title,
                         QString* userName, QString* password);
  ~LoginInputBox();

private slots:
  void accept();
  /// Show letters in the password field
  void onShowPassword(int checkState);

private:
  Ui::LoginInputBox*  ui;
  QString*            _userName;
  QString*            _password;
};