#include <QWidget>
#include <memory>

namespace Ui { class MailInbox; }
class QAbstractItemModel;
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

      void setModel( QAbstractItemModel* m, InboxType type = Inbox );

   private:
      std::unique_ptr<Ui::MailInbox> ui;
      InboxType                      _type;
};
