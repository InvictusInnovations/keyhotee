
#include "MailViewer.hpp"
#include "KeyhoteeMainWindow.hpp"
#include "../ui_MailViewer.h"
#include <QToolBar>

//DLNFIX move this to utility function file
QString makeContactListString(std::vector<fc::ecc::public_key> key_list);


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

#include "MailboxModel.hpp" //for MessageHeader
void MailViewer::displayMailMessages(std::vector<MessageHeader> msgs)
{
   if (msgs.size() == 1)
   {
      MessageHeader& msg = msgs[0];
      QString formatted_date = msg.date_sent.toString(Qt::DefaultLocaleShortDate);
      ui->date_label->setText(formatted_date);
      ui->from_label->setText(msg.from);
      if (msg.to_list.size())
      {
         ui->to_prefix->show();
         ui->to_label->show();
         ui->to_label->setText( makeContactListString(msg.to_list) );
      }
      else
      {
         ui->to_prefix->hide();
         ui->to_label->hide();
      }
      if (msg.cc_list.size())
      {
         ui->cc_prefix->show();
         ui->cc_label->show();
         ui->cc_label->setText( makeContactListString(msg.cc_list) );
      }
      else
      {
         ui->cc_prefix->hide();
         ui->cc_label->hide();
      }
      //TODO: add to and cc lists
      ui->subject_label->setText(msg.subject);
      ui->message_content->setHtml(msg.body);
   }
   else
   {
   //TODO: show summary display when multiple messages selected
   }
}

