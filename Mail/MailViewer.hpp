#include <QWidget>
#include <memory>

namespace Ui { class MailViewer; }

class QToolBar;
class QAction;

class MailViewer : public QWidget
{
   Q_OBJECT
   public:
       MailViewer( QWidget* parent = nullptr );
      ~MailViewer();

   private:
      QToolBar*                       message_tools;
      QAction*                        reply_all;
      QAction*                        reply;
      QAction*                        forward;
      QAction*                        delete_mail;
      std::unique_ptr<Ui::MailViewer> ui;
};
