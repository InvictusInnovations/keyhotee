#include <QWidget>
#include <memory>

namespace Ui { class Mailbox; }
class QAbstractItemModel;
class QItemSelection;
class MailboxModel;
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

       Mailbox( QWidget* parent = nullptr );
      ~Mailbox();

      void setModel( MailboxModel* model, InboxType type = Inbox );
      void searchEditChanged(QString search_string);

   private:
      void setupActions();
      QModelIndex getSelectedMail();
      void showCurrentMail(const QModelIndex &selected, const QModelIndex &deselected);
      void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

      void replyMail(bool reply_all);
      void onReplyMail()    { replyMail(false); }
      void onReplyAllMail() { replyMail(true); }

      void onForwardMail();
      void onDeleteMail();

      QSortFilterProxyModel* sortedModel();


      std::unique_ptr<Ui::Mailbox> ui;
      InboxType                      _type;
      MailboxModel*                  _sourceModel;

      QAction*                        reply_mail;
      QAction*                        reply_all_mail;
      QAction*                        forward_mail;
      QAction*                        delete_mail;
};
