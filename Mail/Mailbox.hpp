#include <KeyhoteeMainWindow.hpp>
#include <QWidget>
#include <memory>

namespace Ui { class Mailbox; }

class IMailProcessor;
class MailboxModel;

class QAbstractItemModel;
class QItemSelection;
class QSortFilterProxyModel;

class Mailbox : public QWidget
{
  Q_OBJECT
public:
  enum InboxType
    {
    Inbox,
    Drafts,
    Sent
    };

  Mailbox(QWidget* parent = nullptr);
  virtual ~Mailbox();

  // void setModel(IMailProcessor& mailProcessor, MailboxModel* model, InboxType type = Inbox);
  void initial(IMailProcessor& mailProcessor, MailboxModel* model, InboxType type, KeyhoteeMainWindow* parentKehoteeMainW);
  void searchEditChanged(QString search_string);

  bool isShowDetailsHidden();
  /// Allows to explicitly reread currently displayed message in the preview pane.
  void refreshMessageViewer();
  bool isAttachmentSelected () const;
  void saveAttachment ();

private slots:
  void onDoubleClickedItem(QModelIndex);

private:
  enum ReplyType { reply, reply_all, forward };
  void setupActions();
  QModelIndex getSelectedMail();
  void showCurrentMail(const QModelIndex &selected, const QModelIndex &deselected);
  void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

  void duplicateMail(ReplyType);
  void onReplyMail()
    {
    duplicateMail(ReplyType::reply);
    }

  void onReplyAllMail()
    {
    duplicateMail(ReplyType::reply_all);
    }

  void onForwardMail()
    {
    duplicateMail(ReplyType::forward);
    }

public slots:
  void onDeleteMail();
  void on_actionShow_details_toggled(bool checked);
private:
  QSortFilterProxyModel* sortedModel();
  /// Don't change 'ui' declaration since it breaks QTCreator tools 
  Ui::Mailbox*                 ui;
  InboxType                    _type;
  MailboxModel*                _sourceModel;
  IMailProcessor*              _mailProcessor;
  QAction*                     reply_mail;
  QAction*                     reply_all_mail;
  QAction*                     forward_mail;
  QAction*                     delete_mail;
  bool                        _attachmentSelected;
};
