
#include "MailViewer.hpp"
#include "../ui_MailViewer.h"
#include <QToolBar>

MailViewer::MailViewer( QWidget* parent )
: ui( new Ui::MailViewer() )
{
   ui->setupUi( this );

   message_tools = new QToolBar( ui->toolbar_container ); 
   reply = new QAction( QIcon( ":/images/mail_reply.png"), tr( "Reply"), this );
   reply_all = new QAction( QIcon( ":/images/mail_reply_all.png"), tr( "Reply All"),this );
   forward = new QAction( QIcon( ":/images/mail_forward.png"), tr("Forward"), this);
   delete_mail = new QAction(QIcon( ":/images/delete_icon.png"), tr( "Delete" ), this);

   message_tools->addAction( reply );
   message_tools->addAction( reply_all );
   message_tools->addAction( forward );

   QWidget* spacer = new QWidget(message_tools);
   spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
   message_tools->addWidget(spacer);

   message_tools->addAction( delete_mail );

   QGridLayout* grid_layout = new QGridLayout(ui->toolbar_container);
   grid_layout->setContentsMargins( 0,0,0,0);
   grid_layout->setSpacing(0);
   ui->toolbar_container->setLayout(grid_layout);
   grid_layout->addWidget(message_tools,0,0);
}

MailViewer::~MailViewer()
{
}

#include "InboxModel.hpp" //for MessageHeader
void MailViewer::displayMailMessages(std::vector<MessageHeader> msgs)
{
   if (msgs.size() == 1)
   {
      auto msg = msgs[0];
      QString formatted_date = msg.date_sent.toString(Qt::DefaultLocaleShortDate);
      ui->date_label->setText(formatted_date);
      ui->from_label->setText(msg.from);
      ui->subject_label->setText(msg.subject);
      ui->message_content->setHtml(msg.body);
   }
   else
   {
   //TODO: show summary display when multiple messages selected
   }
}
