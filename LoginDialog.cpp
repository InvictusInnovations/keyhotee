#include "LoginDialog.hpp"

#include "ui_LoginDialog.h"

#include "KeyhoteeApplication.hpp"
#include <QMessageBox>

#include <bts/application.hpp>

#include <fc/log/logger.hpp>
#include <fc/thread/thread.hpp>


LoginDialog::LoginDialog(TKeyhoteeApplication& mainApp, QWidget* parent)
  : QDialog(parent),
  ui(new Ui::LoginDialog),
  _mainApp(mainApp)
{
  ui->setupUi(this);
  ui->password->setFocus();
  connect(ui->login, &QPushButton::clicked, this, &LoginDialog::onLogin);
  connect(ui->newProfile, &QPushButton::clicked, this, &LoginDialog::onNew);
  connect(ui->quit, &QPushButton::clicked, this, &LoginDialog::onQuit);

  auto profiles = bts::application::instance()->get_profiles();
  for(const auto& profileName : profiles)
  {
    QString profileItem = QString::fromStdWString(profileName);
    /// FIXME - fc::log cannot get std::wstring
    wlog( "profiles ${p}", ("p", profileItem.toStdString()) );
    ui->profileSelection->addItem(profileItem);
  }
}

LoginDialog::~LoginDialog()
{
  delete ui;
}

void LoginDialog::onLogin()
{
  try
  {
    password = ui->password->text().toStdString();
    auto profileName = ui->profileSelection->currentText().toStdWString();
    auto profile = bts::application::instance()->load_profile(profileName,password);
    if (profile)
      accept();
  }
  catch (const fc::db_in_use_exception& e)
  {
    elog("error ${w}", ("w", e.to_detail_string()) );
    QMessageBox::warning(this,tr("Keyhotee login"), tr("Unable to load profile: ") +
      ui->profileSelection->currentText());
  }
  catch (const fc::exception& e)
  {
    elog("error ${w}", ("w", e.to_detail_string()) );
   //TODO: We need to display a message to user when profile load fails because
   //      it can't be opened (versus just a bad password specified). One reason
   //      a profile load can fail is because it's already locked (opened by another
   //      Keyhotee process, for example). Unfortunately displaying such a message
   //      requires untangling the exception thrown, so leaving this as-is for now.
   // QMessageBox::warning(this,tr("Unable to load profile"),e.to_detail_string().c_str());
  }
  ui->password->setText(QString());
  ui->password->setFocus();
  shake();
}

void LoginDialog::onNew()
{
   close();
   _mainApp.displayProfileWizard();
}

void LoginDialog::shake()
{
  move(pos() + QPoint(10, 0) );
  fc::usleep(fc::microseconds(50 * 1000) );
  move(pos() + QPoint(-20, 0) );
  fc::usleep(fc::microseconds(50 * 1000) );
  move(pos() + QPoint(20, 0) );
  fc::usleep(fc::microseconds(50 * 1000) );
  move(pos() + QPoint(-10, 0) );
}

void LoginDialog::onQuit()
{
  _mainApp.quit();
  reject();
}

