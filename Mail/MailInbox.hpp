#include <QWidget>
#include <memory>

namespace Ui { class MailInbox; }
class QAbstractItemModel;
class MailInbox : public QWidget
{
   Q_OBJECT
   public:
       MailInbox( QWidget* parent = nullptr );
      ~MailInbox();

      void setModel( QAbstractItemModel* m );

   private:
      std::unique_ptr<Ui::MailInbox> ui;
};
