#ifndef __ATOPLEVELWINDOWSCONTAINER_HPP
#define __ATOPLEVELWINDOWSCONTAINER_HPP

#include "qtreusable/selfsizingmainwindow.h"

namespace Ui { class ATopLevelWindowsContainer; }

class QMenu;
class QActionGroup;

class ATopLevelWindowsContainer :
  public SelfSizingMainWindow
{
public:
  ATopLevelWindowsContainer(QWidget *parent = 0);
  virtual ~ATopLevelWindowsContainer(void);

  void registration(QAction *newAction);
  void unRegistration(QAction *newAction);

  void setMenuWindow(QMenu* menuWindow);

private:
  bool closeAllWindows(void);

private slots:
  void onNextWindow();
  void onPrevWindow();
  void onActiveWindow(QAction* action);

protected:
  virtual void closeEvent(QCloseEvent *event);

private:
  QList<QAction*>   listQActions;
  QMenu*            menuWindow;
  QActionGroup*     menuGroup;
  int               currentWindowIndex;
};

#endif /// __ATOPLEVELWINDOWSCONTAINER_HPP
