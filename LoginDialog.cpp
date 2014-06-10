#include "LoginDialog.hpp"

#include "ui_LoginDialog.h"

#include "KeyhoteeApplication.hpp"
#include <QMessageBox>
#include <QSettings>

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

#ifdef Q_OS_MAC
  ui->frame->setStyleSheet(QStringLiteral("image: url(:/images/keyhotee.icns);"));
#else
  ui->frame->setStyleSheet(QStringLiteral("image: url(:/images/keyhotee.png);"));
#endif

  ui->frame->setFrameShape(QFrame::NoFrame);
  connect(ui->login, &QPushButton::clicked, this, &LoginDialog::onLogin);
  connect(ui->newProfile, &QPushButton::clicked, this, &LoginDialog::onNew);
  connect(ui->quit, &QPushButton::clicked, this, &LoginDialog::onQuit);

  auto profile_name_to_load = _mainApp.getLoadedProfileName();
  _selected_profile_ok = false;

  auto profiles = bts::application::instance()->get_profiles();
  for(const auto& profileName : profiles)
  {
    QString profileItem = QString::fromStdWString(profileName);
    /// FIXME - fc::log cannot get std::wstring
    wlog( "profiles ${p}", ("p", profileItem.toStdString()) );
    ui->profileSelection->addItem(profileItem);
    if(profile_name_to_load == profileItem)
    {
      ui->profileSelection->setCurrentIndex(ui->profileSelection->count()-1);
      _selected_profile_ok = true;
    }
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
    {
      _mainApp.setLoadedProfileName(ui->profileSelection->currentText());
      QSettings settings("Invictus Innovations", "Keyhotee");
      settings.setValue("last_profile", ui->profileSelection->currentText());
      accept();
    }
  }
  catch (const db_in_use_exception& e)
  {
    elog("error ${w}", ("w", e.to_detail_string()) );
    QMessageBox::warning(this,tr("Keyhotee login"), tr("Unable to load profile: ") +
      ui->profileSelection->currentText());
  }
  catch (const fc::parse_error_exception& e)
  {
    elog("error ${w}", ("w", e.to_detail_string()));
    QMessageBox::warning(this, tr("Syntax error"), tr("Unable to load profile: ") +
      ui->profileSelection->currentText() + tr(".\nThe file format is invalid."));
  }
  catch (const fc::exception& e)
  {
    elog("error ${w}", ("w", e.to_detail_string()) );
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

