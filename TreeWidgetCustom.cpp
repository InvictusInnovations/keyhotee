#include "TreeWidgetCustom.hpp"

#include "ch/ModificationsChecker.hpp"

#include <QEvent>
#include <QKeyEvent>
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

  _accept_request = new QAction(QIcon(":/images/request_accept.png"), tr("Accept"), this);
  _deny_request = new QAction(QIcon(":/images/request_deny.png"), tr("Deny"), this);
  _deny_request->setShortcut(Qt::Key_Delete);
  _block_request = new QAction(QIcon(":/images/request_block.png"), tr("Block"), this);
  _menu_requests.addAction(_accept_request);
  _menu_requests.addAction(_deny_request);
  _menu_requests.addAction(_block_request);
  connect(_accept_request, &QAction::triggered, this, &TreeWidgetCustom::onAcceptRequest);
  connect(_deny_request, &QAction::triggered, this, &TreeWidgetCustom::onDenyRequest);
  connect(_block_request, &QAction::triggered, this, &TreeWidgetCustom::onBlockRequest);

  _deny_multi_request = new QAction(QIcon(":/images/request_deny.png"), tr("Deny All"), this);
  _deny_multi_request->setShortcut(Qt::Key_Delete);
  _menu_multi_requests.addAction(_deny_multi_request);
  _menu_multi_requests.addAction(_block_request);
  connect(_deny_multi_request, &QAction::triggered, this, &TreeWidgetCustom::onDenyMultiRequest);
  }

TreeWidgetCustom::~TreeWidgetCustom(){}

bool TreeWidgetCustom::viewportEvent(QEvent *event)
  {
  if (_modificationsChecker)
    {
    //Don't change selection if you can't continue (data invalid or modified)
    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonDblClick ||
        event->type() == QEvent::KeyPress)
      {
        if (_modificationsChecker->canContinue())
          return QTreeWidget::viewportEvent(event);
        else
          return false;
      }
    }

  return QTreeWidget::viewportEvent(event);
  }

void TreeWidgetCustom::keyPressEvent(QKeyEvent *event)
{
  if (_modificationsChecker)
  {
    if (event->key() == Qt::Key_Down || event->key() == Qt::Key_Up ||
        event->key() == Qt::Key_Right || event->key() == Qt::Key_Left ||
        event->key() == Qt::Key_PageDown || event->key() == Qt::Key_PageUp ||
        event->key() == Qt::Key_Home || event->key() == Qt::Key_End)
    {
      if (! _modificationsChecker->canContinue())
        return;
    }
  }

  // call the default implementation
  QTreeWidget::keyPressEvent(event);
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
    case RequestItem:
      if(item->data(0, Qt::UserRole).toBool())
        _menu_multi_requests.exec(globalPos);
      else
        _menu_requests.exec(globalPos);
        break;
    default:
        break;
    }
  }

void TreeWidgetCustom::onRemoveContact ()
  {
    emit itemContactRemoved (*_currentItem);
  }

void TreeWidgetCustom::onAcceptRequest()
{
  emit itemContextAcceptRequest(_currentItem);
}

void TreeWidgetCustom::onDenyRequest()
{
  emit itemContextDenyRequest(_currentItem);
}

void TreeWidgetCustom::onBlockRequest()
{
  emit itemContextBlockRequest(_currentItem);
}

void TreeWidgetCustom::onDenyMultiRequest()
{
  emit itemContextDenyRequest(_currentItem);
}
