#include <QWidget>
#include <memory>

namespace Ui { class MailInbox; }
class QAbstractItemModel;
class QItemSelection;

class MailInbox : public QWidget
{
   Q_OBJECT
   public:
      enum InboxType
      {
          Inbox,
          Drafts,
          Sent
      };

       MailInbox( QWidget* parent = nullptr );
      ~MailInbox();

      void setModel( QAbstractItemModel* model, InboxType type = Inbox );
   private:
      void setupActions();
      void showCurrentMail(const QModelIndex &selected, const QModelIndex &deselected);
      void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
      void onReplyMail();
      void onReplyAllMail();
      void onForwardMail();
      void onDeleteMail();

      std::unique_ptr<Ui::MailInbox> ui;
      InboxType                      _type;

      QAction*                        reply_mail;
      QAction*                        reply_all_mail;
      QAction*                        forward_mail;
      QAction*                        delete_mail;
};
