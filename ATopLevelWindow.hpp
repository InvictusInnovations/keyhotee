#ifndef __ATOPLEVELWINDOW_HPP
#define __ATOPLEVELWINDOW_HPP

#include "qtreusable/selfsizingmainwindow.h"

namespace Ui { class ATopLevelWindow; }

class QAction;
class ATopLevelWindowsContainer;

class ATopLevelWindow :
  public SelfSizingMainWindow
{
public:
  ATopLevelWindow(ATopLevelWindowsContainer *parent = 0);
  virtual ~ATopLevelWindow(void);

  void setWindowTitle(const QString& title_string);

private:
  void SetActionText(QString string);

private:
  QAction* actionMenu;
  ATopLevelWindowsContainer* parentWin;

protected:
  virtual void closeEvent(QCloseEvent *event);
};

#endif __ATOPLEVELWINDOW_HPP
