
#include "MailViewer.hpp"
#include "KeyhoteeMainWindow.hpp"
#include "../ui_MailViewer.h"
#include <QToolBar>

MailViewer::MailViewer( QWidget* parent )
: ui( new Ui::MailViewer() )
{
   ui->setupUi( this );
   message_tools = new QToolBar( ui->toolbar_container ); 
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
      MessageHeader& msg = msgs[0];
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

