#include "ATopLevelWindowsContainer.hpp"

#include <QMenu>
#include <QKeyEvent>
#include <qshortcut.h>
#include <qactiongroup.h>

ATopLevelWindowsContainer::ATopLevelWindowsContainer(QWidget *parent):
  SelfSizingMainWindow(parent)
{
  QShortcut *shortcutNext = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Tab), this);
  shortcutNext->setContext(Qt::ApplicationShortcut);
  connect(shortcutNext, &QShortcut::activated, this, &ATopLevelWindowsContainer::onNextWindow);

  QShortcut *shortcutPrev = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab), this);
  shortcutPrev->setContext(Qt::ApplicationShortcut);
  connect(shortcutPrev, &QShortcut::activated, this, &ATopLevelWindowsContainer::onPrevWindow);

  menuGroup = new QActionGroup(this);

  currentWindowIndex = 0;
}

ATopLevelWindowsContainer::~ATopLevelWindowsContainer(void)
{
}

void ATopLevelWindowsContainer::registration(QAction *newAction)
{
  listQActions.append(newAction);
  menuWindow->addAction(newAction);
  menuGroup->addAction(newAction);
  newAction->setChecked(true);
  currentWindowIndex = listQActions.count()-1;
}

void ATopLevelWindowsContainer::unRegistration(QAction *newAction)
{
  listQActions.removeOne(newAction);
  menuWindow->removeAction(newAction);
  menuGroup->removeAction(newAction);
  currentWindowIndex = 0;
}

void ATopLevelWindowsContainer::setMenuWindow(QMenu* menu)
{
  menuWindow = menu;
  connect(menuWindow, &QMenu::triggered, this, &ATopLevelWindowsContainer::onActiveWindow);
}

void ATopLevelWindowsContainer::onNextWindow()
{
//  menuWindow->exec(this->mapToGlobal(10, 10));

  currentWindowIndex++;
  if( currentWindowIndex >= listQActions.count() )
    currentWindowIndex = 0;
  onActiveWindow(listQActions[currentWindowIndex]);
}

void ATopLevelWindowsContainer::onPrevWindow()
{
  currentWindowIndex--;
  if( currentWindowIndex < 0 )
    currentWindowIndex = listQActions.count() - 1;
  onActiveWindow(listQActions[currentWindowIndex]);
}

void ATopLevelWindowsContainer::onActiveWindow(QAction* action)
{
  action->parentWidget()->setWindowState( (windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
  action->parentWidget()->raise();
  action->parentWidget()->activateWindow();
  action->setChecked(true);
  currentWindowIndex = listQActions.indexOf(action);
}

bool ATopLevelWindowsContainer::closeAllWindows()
{
  bool closeOK = true;
  for(int i=listQActions.size()-1; i>0; i--)
    closeOK &= listQActions[i]->parentWidget()->close();

  return closeOK;
}

void ATopLevelWindowsContainer::closeEvent(QCloseEvent *event)
{
  if(listQActions.size() > 1)
    if(!closeAllWindows())
      event->ignore();
}
