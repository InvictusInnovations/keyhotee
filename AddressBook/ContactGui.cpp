#include "ContactGui.hpp"

#include "ContactView.hpp"
#include "KeyhoteeMainWindow.hpp"

#include <QDateTime>
#include <QString>
#include <QTreeWidgetItem>

void ContactGui::setUnreadMsgCount(unsigned int count)
{
  _unread_msg_count = count;
  updateTreeItemDisplay();
}

bool ContactGui::isChatVisible()
{
  return getKeyhoteeWindow()->isSelectedContactGui(this) && _view->isChatSelected();
}

void ContactGui::receiveChatMessage(const QString& from, const QString& msg, const QDateTime& dateTime)
{
  _view->appendChatMessage(from, msg, dateTime);
  if (!isChatVisible())
    setUnreadMsgCount(_unread_msg_count + 1);
}

void ContactGui::updateTreeItemDisplay()
{
  if(_view->getContact().isBlocked())
    _tree_item->setHidden(true);
  else
  {
    QString display_text;
    QString name = _view->getContact().getLabel();
    if(_unread_msg_count)
      display_text = QString("%1 (%2)").arg(name).arg(_unread_msg_count);
    else
      display_text = name;
    _tree_item->setText(0, display_text);
    _tree_item->setHidden(false);
  }
}
