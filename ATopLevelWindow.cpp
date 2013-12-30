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

  //DLNFIX Q&D workaround as I needed to test saving draft mail messages. Greg, please make a better fix
  #if 0
  ((ATopLevelWindowsContainer*)parent)->registration(actionMenu);
  #endif

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
  //DLNFIX Q&D workaround as I need edto test saving draft mail messages. Greg, please make a better fix
#if 0
  ((ATopLevelWindowsContainer*)this->parent())->unRegistration(actionMenu);
#endif
}
