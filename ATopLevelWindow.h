#ifndef __ATOPLEVELWINDOW_HPP
#define __ATOPLEVELWINDOW_HPP

#include "qtreusable/selfsizingmainwindow.h"
#include "ATopLevelWindowsContainer.h"

namespace Ui { class ATopLevelWindow; }

class QAction;

class ATopLevelWindow :
  public SelfSizingMainWindow
{
public:
  ATopLevelWindow(QWidget *parent = 0);
  ~ATopLevelWindow(void);

  void setWindowTitle(const QString& title_string);

private:
  void SetActionText(QString string);

private:
  QAction* actionMenu;

protected:
  void closeEvent(QCloseEvent *event);

//Q_SIGNALS:
//  void windowOpened(QAction* actionMenu);
};

#endif __ATOPLEVELWINDOW_HPP
