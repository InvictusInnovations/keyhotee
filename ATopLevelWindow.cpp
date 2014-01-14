#include "ATopLevelWindow.hpp"
#include "ATopLevelWindowsContainer.hpp"

#include <QAction>
#include <QKeyEvent>

ATopLevelWindow::ATopLevelWindow(ATopLevelWindowsContainer *parent) :
  SelfSizingMainWindow()
{
  parentWin = parent;

  actionMenu = new QAction(tr("[*]"), this);
  actionMenu->setCheckable(true);

  parentWin->registration(actionMenu);
}

ATopLevelWindow::~ATopLevelWindow(void)
{
}

void ATopLevelWindow::setWindowTitle(const QString& title_string)
{
  QWidget::setWindowTitle(title_string);
  SetActionText(title_string);
}

void ATopLevelWindow::SetActionText(QString string)
{
  actionMenu->setText(string);
}

void ATopLevelWindow::closeEvent(QCloseEvent *event)
{
  parentWin->unRegistration(actionMenu);
}

bool ATopLevelWindow::event(QEvent *event)
{
  if(event->type() == QEvent::WindowActivate)
  {
    actionMenu->setChecked(true);
  }

  return QMainWindow::event(event);
}

