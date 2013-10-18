#include <QWidget>
#include <memory>
#include <bts/bitchat/bitchat_private_message.hpp>
#include "MessageHeader.hpp"

namespace Ui { class MailViewer; }

class QToolBar;
class QAction;

class MailViewer : public QWidget
{
   Q_OBJECT
   public:
       MailViewer( QWidget* parent = nullptr );
      ~MailViewer();
      void displayMailMessages(std::vector<MessageHeader>);

   private:
      QToolBar*                       message_tools;
      QAction*                        reply_all;
      QAction*                        reply;
      QAction*                        forward;
      QAction*                        delete_mail;
      std::unique_ptr<Ui::MailViewer> ui;
};
