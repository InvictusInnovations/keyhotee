#pragma once
#include <QTreeWidget>
#include <QMenu>

class IModificationsChecker;

enum MailboxChildren
  {
  Inbox,
  Drafts,
  Outbox,
  Sent,
  Spam
  };

enum SidebarItemTypes
  {
  IdentityItem  = 1000 + 2,
  MailboxItem   = 1000 + 3,
  ContactItem   = 1000 + 4,
  RequestItem   = 1000 + 5,
  WalletItem    = 1000 + 6
  };

class TreeWidgetCustom : public QTreeWidget
{
    Q_OBJECT
public:
  explicit TreeWidgetCustom(QWidget *parent = nullptr);
  virtual ~TreeWidgetCustom();
  void setModificationsChecker (IModificationsChecker*);

protected:
  virtual bool viewportEvent(QEvent *event) override;
  virtual void keyPressEvent(QKeyEvent *event) override;
  void showContextMenu(QTreeWidgetItem* item, const QPoint& globalPos);

private slots:
  void onCustomContextMenuRequested(const QPoint& pos);
  void onRemoveContact ();
  void onAcceptRequest();
  void onDenyRequest();
  void onBlockRequest();
  void onDenyMultiRequest();

Q_SIGNALS:
  void itemContactRemoved (QTreeWidgetItem& item);
  void itemContextAcceptRequest (QTreeWidgetItem* item);
  void itemContextBlockRequest (QTreeWidgetItem* item);
  void itemContextDenyRequest (QTreeWidgetItem* item);

private:
  IModificationsChecker*    _modificationsChecker;
  QAction*                  _removeContact;
  QMenu                     _menuContacts;
  QAction*                  _accept_request;
  QAction*                  _deny_request;
  QAction*                  _block_request;
  QMenu                     _menu_requests;
  QAction*                  _deny_multi_request;
  QMenu                     _menu_multi_requests;
  QTreeWidgetItem*          _currentItem;
};