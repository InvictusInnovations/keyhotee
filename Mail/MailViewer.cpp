
#include "MailViewer.hpp"
#include "KeyhoteeMainWindow.hpp"
#include "ui_MailViewer.h"
#include <QToolBar>
#include "MailboxModel.hpp"
#include <QImageReader>

//DLNFIX move this to utility function file
QString makeContactListString(std::vector<fc::ecc::public_key> key_list);


MailViewer::MailViewer( QWidget* parent )
: QWidget(parent), ui( new Ui::MailViewer() )
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

void MailViewer::displayMailMessage(const QModelIndex& index, MailboxModel* mailbox)
{ 
   
   //TODO: later, possibly set a timer and only mark as read if still displaying
   //      this message when timer expires?
   mailbox->markMessageAsRead(index);
   MessageHeader msg;
   mailbox->getFullMessage(index,msg);
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
   displayAttachments(msg);
}

void MailViewer::displayAttachments(const MessageHeader& msg)
{   
#if 0 //code doesn't yet work
   QTextDocument* textDocument = ui->message_content->document();
   QImage image;
   int i = 1;
   foreach (const bts::bitchat::attachment& attachment, msg.attachments)
      {
      image.fromData(attachment.body.data());
      QUrl url(QString("attachment_image_%1").arg(i++));
      textDocument->addResource( QTextDocument::ImageResource, url, QVariant ( image ) );
      QTextCursor cursor = ui->message_content->textCursor();
      QTextImageFormat imageFormat;
      imageFormat.setWidth( image.width() );
      imageFormat.setHeight( image.height() );
      imageFormat.setName( url.toString() );
      cursor.insertImage(imageFormat);
      }
#endif
}

#if 0
void MailViewer::displayMailMessages(QModelIndexList indexes,QItemSelectionModel* selection_model)
{
   QSortFilterProxyModel* model = dynamic_cast<QSortFilterProxyModel*>(ui->inbox_table->model());
   foreach (QModelIndex index, items) 
   {
      auto sourceModelIndex = model->mapToSource(index);
      _sourceModel->getFullMessage(sourceModelIndex,message_header);
      msgs.push_back(message_header);
   }

   {
   //TODO: show summary display when multiple messages selected
   }
}
#endif

