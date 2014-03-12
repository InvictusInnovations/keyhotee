#include "TableViewCustom.hpp"

#include "ch/ModificationsChecker.hpp"

#include <QEvent>
#include <QKeyEvent>

TableViewCustom::TableViewCustom(QWidget* parent)
  : QTableView(parent),
  _modificationsChecker (nullptr)
  {
  }

TableViewCustom::~TableViewCustom(){}

bool TableViewCustom::viewportEvent(QEvent *event)
  { 
  //Don't change selection if you can't continue (data invalid or modified)
  if (_modificationsChecker)
    {
    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonDblClick ||
        event->type() == QEvent::KeyPress)
		  {
        if (_modificationsChecker->canContinue())
          return QTableView::viewportEvent(event);
        else
          return false;
		  }
    }

  return QTableView::viewportEvent(event);
  }

void TableViewCustom::keyPressEvent(QKeyEvent *event)
{
  if (_modificationsChecker)
  {
    if (event->key() == Qt::Key_Down || event->key() == Qt::Key_Up ||
        event->key() == Qt::Key_PageDown || event->key() == Qt::Key_PageUp ||
        event->key() == Qt::Key_Home || event->key() == Qt::Key_End ||
        event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab)
    {
      if (! _modificationsChecker->canContinue())
        return;
    }
  }

  // call the default implementation
  QTableView::keyPressEvent(event);
}

void TableViewCustom::setModificationsChecker (IModificationsChecker* modificationsChecker)
  {
  _modificationsChecker = modificationsChecker;
  }