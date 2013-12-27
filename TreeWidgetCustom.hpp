#pragma once
#include <QTreeWidget>
#include <QMenu>

class IModificationsChecker;

enum MailboxChildren
  {
  Inbox,
  Drafts,
  Sent
  };

enum SidebarItemTypes
  {
  IdentityItem = 2,
  MailboxItem = 3,
  ContactItem = 4
  };

enum WalletsChildren
  {
  Bitcoin,
  BitShares,
  Litecoin
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
  void showContextMenu(QTreeWidgetItem* item, const QPoint& globalPos);
  
private slots:
    void onCustomContextMenuRequested(const QPoint& pos);
    void onRemoveContact ();

Q_SIGNALS:
    void itemContactRemoved (QTreeWidgetItem& item);

private:
  IModificationsChecker*    _modificationsChecker;
  QAction*                  _removeContact;
  QMenu                     _menuContacts;
  QTreeWidgetItem*          _currentItem;
};