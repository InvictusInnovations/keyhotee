#include <QWidget>
#include <memory>

namespace Ui { class Mailbox; }
class QAbstractItemModel;
class QItemSelection;

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

      void setModel( QAbstractItemModel* model, InboxType type = Inbox );
   private:
      void setupActions();
      void showCurrentMail(const QModelIndex &selected, const QModelIndex &deselected);
      void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
      void onReplyMail();
      void onReplyAllMail();
      void onForwardMail();
      void onDeleteMail();

      std::unique_ptr<Ui::Mailbox> ui;
      InboxType                      _type;

      QAction*                        reply_mail;
      QAction*                        reply_all_mail;
      QAction*                        forward_mail;
      QAction*                        delete_mail;
};
