#include "TableViewCustom.hpp"
#include "ch/ModificationsChecker.hpp"
#include <QEvent>

TableViewCustom::TableViewCustom(QWidget* parent)
  : QTableView(parent),
  _modificationsChecker (nullptr)
  {
  }

TableViewCustom::~TableViewCustom(){}

bool TableViewCustom::viewportEvent(QEvent *event)
  { 
  //Don't change selection if you can't continue (data invalid or modyfied)
  if (_modificationsChecker)
    {
    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonDblClick ||
        event->type() == QEvent::KeyPress ||
        event->type() == QEvent::MouseMove)
		  {
        if (_modificationsChecker->canContinue())
          return QTableView::viewportEvent(event);
        else
          return false;
		  }
    }

  return QTableView::viewportEvent(event);
  }

void TableViewCustom::setModificationsChecker (IModificationsChecker* modificationsChecker)
  {
  _modificationsChecker = modificationsChecker;
  }