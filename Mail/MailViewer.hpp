#include <QWidget>
#include <memory>

namespace Ui { class MailViewer; }

class MailViewer : public QWidget
{
   Q_OBJECT
   public:
       MailViewer( QWidget* parent = nullptr );
      ~MailViewer();

   private:
      std::unique_ptr<Ui::MailViewer> ui;
};
