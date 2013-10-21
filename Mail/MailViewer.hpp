#include <QWidget>
#include <memory>
#include <bts/bitchat/bitchat_private_message.hpp>
#include "MessageHeader.hpp"

namespace Ui { class MailViewer; }
class QToolBar;

class MailViewer : public QWidget
{
   Q_OBJECT
   public:
       MailViewer( QWidget* parent = nullptr );
      ~MailViewer();
      void displayMailMessages(std::vector<MessageHeader>);

      QToolBar*                       message_tools;
   private:
      std::unique_ptr<Ui::MailViewer> ui;
};
