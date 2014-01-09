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
  for( uint32_t i = 0; i < profiles.size(); ++i )
  {
      wlog( "profiles ${p}", ("p",profiles[i]) );
      ui->profileSelection->insertItem( i, profiles[i].c_str() );
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
    auto profile_name = ui->profileSelection->currentText().toStdString();
    auto profile = bts::application::instance()->load_profile(profile_name,password);
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

