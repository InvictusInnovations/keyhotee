#include "TreeWidgetCustom.hpp"
#include <QEvent>
#include "ch/ModificationsChecker.hpp"

TreeWidgetCustom::TreeWidgetCustom(QWidget* parent)
  : QTreeWidget(parent),
  _modificationsChecker (nullptr)
  {
  }

TreeWidgetCustom::~TreeWidgetCustom(){}

bool TreeWidgetCustom::viewportEvent(QEvent *event)
  {
  if (_modificationsChecker)
    {
    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonDblClick ||
        event->type() == QEvent::KeyPress ||
        event->type() == QEvent::MouseMove)
		  {
        if (_modificationsChecker->canContinue())
          return QTreeWidget::viewportEvent(event);
        else
          return false;
		  }
    }

  return QTreeWidget::viewportEvent(event);
  }

void TreeWidgetCustom::setModificationsChecker (IModificationsChecker* modificationsChecker)
  {
  _modificationsChecker = modificationsChecker;
  }
