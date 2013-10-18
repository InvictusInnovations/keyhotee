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
      void showCurrentMail(const QModelIndex &selected, const QModelIndex &deselected);
      void showSelectedMail(const QItemSelection& selected, const QItemSelection& deselected);

      std::unique_ptr<Ui::MailInbox> ui;
      InboxType                      _type;
};
