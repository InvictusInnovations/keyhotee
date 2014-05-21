#include "LoginInputBox.hpp"
#include "ui_LoginInputBox.h"

LoginInputBox::LoginInputBox(QWidget *parent, const QString& title, 
                             QString* userName, QString* password) :
    QDialog(parent),
    ui(new Ui::LoginInputBox),
    _userName(userName),
    _password(password)
{
    ui->setupUi(this);
    connect(ui->showPassword, SIGNAL(stateChanged(int)), this, SLOT(onShowPassword(int)));
    setWindowTitle(title);
}

LoginInputBox::~LoginInputBox()
{
    delete ui;
}

void LoginInputBox::accept()
{
  *_userName = ui->userName->text();
  *_password = ui->password->text();

  QDialog::accept();
}

void LoginInputBox::onShowPassword(int checkState)
{
  if (checkState == Qt::Checked)
    ui->password->setEchoMode(QLineEdit::Normal);
  else
    ui->password->setEchoMode(QLineEdit::Password);    
}