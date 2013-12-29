#include "ATopLevelWindow.h"
#include "ATopLevelWindowsContainer.h"

#include <QAction>
#include <QKeyEvent>

ATopLevelWindow::ATopLevelWindow(QWidget *parent) :
  SelfSizingMainWindow(parent)
{
  QMainWindow::setParent(parent);
//  setWindowFlags(Qt::WindowType::Dialog);

  actionMenu = new QAction(tr("[*]"), this);
  actionMenu->setCheckable(true);

  ((ATopLevelWindowsContainer*)parent)->registration(actionMenu);

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
  std::string _string = string.toStdString();

  printf(_string.c_str());

  actionMenu->setText(string);
}

void ATopLevelWindow::closeEvent(QCloseEvent *event)
{
  ((ATopLevelWindowsContainer*)this->parent())->unRegistration(actionMenu);
}
