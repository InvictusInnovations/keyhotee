#include "TreeWidgetCustom.hpp"
#include "ch/ModificationsChecker.hpp"
#include <QEvent>
#include <QTreeWidgetItem>

TreeWidgetCustom::TreeWidgetCustom(QWidget* parent)
  : QTreeWidget(parent),
  _modificationsChecker (nullptr)
  {
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
                  SLOT(onCustomContextMenuRequested(const QPoint&)));  

  _removeContact = new QAction(QIcon(":/images/128x128/contact_info_cancel_edit.png"), tr("Remove"), this);
  _menuContacts.addAction(_removeContact);  
  connect(_removeContact, &QAction::triggered, this, &TreeWidgetCustom::onRemoveContact);
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


void TreeWidgetCustom::onCustomContextMenuRequested(const QPoint& pos) 
  {
  QTreeWidgetItem* item = itemAt(pos);
  if (item) 
    {
    showContextMenu(item, viewport()->mapToGlobal(pos));
    }
  }
 
void TreeWidgetCustom::showContextMenu(QTreeWidgetItem* item, const QPoint& globalPos)
  {
  _currentItem = item;
  switch (item->type()) 
    {
    case ContactItem:
        _menuContacts.exec(globalPos);
        break;
    default:
        break;
    }
  }

void TreeWidgetCustom::onRemoveContact ()
  {
    emit itemContactRemoved (*_currentItem);
  }