#include "selfsizingmainwindow.h"
#include <bitsharesguiprecomp.h>
#include "KeyhoteeMainWindow.hpp"


SelfSizingMainWindow::SelfSizingMainWindow(QWidget *parent) :
  QMainWindow(parent)
  {}

// this should be called in constructor of derived classes after call to setupUi
void SelfSizingMainWindow::readSettings()
  {
  QSettings settings("Invictus Innovations", _settings_file);
  settings.beginGroup(windowTitle());
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("state").toByteArray(), VERSIONNUM);
  settings.endGroup();

  settings.beginGroup("Mail");
  _mailSettings.sortColumnInbox = settings.value("sortColumnInbox", QVariant(0)).toInt();
  _mailSettings.sortOrderInbox = settings.value("sortOrderInbox", QVariant(0)).toInt();
  settings.endGroup();
  }

void SelfSizingMainWindow::writeSettings()
  {
  getKeyhoteeWindow()->setMailSettings (_mailSettings);

  QSettings settings("Invictus Innovations", _settings_file);
  settings.beginGroup(windowTitle());
  settings.setValue("geometry", saveGeometry());
  settings.setValue("state", saveState(VERSIONNUM));
  settings.endGroup();

  settings.beginGroup("Mail");
  settings.setValue("sortColumnInbox", _mailSettings.sortColumnInbox);
  settings.setValue("sortOrderInbox", _mailSettings.sortOrderInbox);
  settings.endGroup();
  }

const MailSettings& SelfSizingMainWindow::getMailSettings() const
{
  return _mailSettings;
}

/*
// KeyhoteeMainWindow::writeSettings ();
void SelfSizingMainWindow::closeEvent(QCloseEvent *event)
  {
  if (okToContinue())
    {
    writeSettings();
    event->accept();
    }
  else
    {
    event->ignore();
    }
  }
*/

