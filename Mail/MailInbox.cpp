#include "MailInbox.hpp"
#include "../ui_MailInbox.h"
#include "InboxModel.hpp"
#include <fc/reflect/variant.hpp>
#include "MailEditor.hpp"
#include <QToolBar>


MailInbox::MailInbox( QWidget* parent )
: ui( new Ui::MailInbox() ),
  _type(Inbox)
{
   ui->setupUi( this );
   setupActions();
}


void MailInbox::showCurrentMail(const QModelIndex &selected,
                                 const QModelIndex &deselected)
{
}

void MailInbox::onSelectionChanged(const QItemSelection &selected,
                                  const QItemSelection &deselected)
{
   QItemSelectionModel* selection_model = ui->inbox_table->selectionModel();
   QModelIndexList items = selection_model->selectedRows();
   //disable reply buttons if more than one email selected
   bool oneEmailSelected = (items.size() == 1);
   reply_mail->setEnabled(oneEmailSelected);
   reply_all_mail->setEnabled(oneEmailSelected);
   forward_mail->setEnabled(oneEmailSelected);
   //display selected email(s) in message preview window
   std::vector<MessageHeader> msgs;
   MessageHeader message_header;
   InboxModel* model = static_cast<InboxModel*>(ui->inbox_table->model());
   foreach (QModelIndex index, items) 
   {
      model->getFullMessage(index,message_header);
      msgs.push_back(message_header);
   }
   ui->current_message->displayMailMessages(msgs);
}

MailInbox::~MailInbox()
{
}

void MailInbox::setModel( QAbstractItemModel* model, InboxType type )
{
   _type = type;
   ui->inbox_table->setModel(model);

   ui->inbox_table->horizontalHeader()->resizeSection( InboxModel::To, 120 );
   ui->inbox_table->horizontalHeader()->resizeSection( InboxModel::Subject, 300 );
   ui->inbox_table->horizontalHeader()->resizeSection( InboxModel::DateReceived, 120 );
   ui->inbox_table->horizontalHeader()->resizeSection( InboxModel::From, 120 );
   ui->inbox_table->horizontalHeader()->resizeSection( InboxModel::DateSent, 120 );
   if( _type == Inbox )
   {
      ui->inbox_table->horizontalHeader()->hideSection( InboxModel::Status );
      ui->inbox_table->horizontalHeader()->hideSection( InboxModel::DateSent );
   }
   if( _type == Sent )
   {
      ui->inbox_table->horizontalHeader()->swapSections( InboxModel::To, InboxModel::From );
      ui->inbox_table->horizontalHeader()->swapSections( InboxModel::DateReceived, InboxModel::DateSent );
      ui->inbox_table->horizontalHeader()->hideSection( InboxModel::DateReceived );
   }
   if( _type == Drafts )
   {
      ui->inbox_table->horizontalHeader()->swapSections( InboxModel::To, InboxModel::From );
      ui->inbox_table->horizontalHeader()->swapSections( InboxModel::DateReceived, InboxModel::DateSent );
      ui->inbox_table->horizontalHeader()->hideSection( InboxModel::DateReceived );
      ui->inbox_table->horizontalHeader()->hideSection( InboxModel::Status );
   }

   ui->inbox_table->horizontalHeader()->setSectionsMovable(true);
   ui->inbox_table->horizontalHeader()->setSortIndicatorShown(false);
   ui->inbox_table->horizontalHeader()->setSectionsClickable(true);
   ui->inbox_table->horizontalHeader()->setHighlightSections(true);

   //connect signals for the new selection model (created by setModel call)
   QItemSelectionModel* inbox_selection_model = ui->inbox_table->selectionModel();
   connect( inbox_selection_model, &QItemSelectionModel::selectionChanged, this, &MailInbox::onSelectionChanged );
   connect( inbox_selection_model, &QItemSelectionModel::currentChanged, this, &MailInbox::showCurrentMail );

   connect( reply_mail, &QAction::triggered, this, &MailInbox::onReplyMail);
   connect( reply_all_mail, &QAction::triggered, this, &MailInbox::onReplyAllMail);
   connect( forward_mail, &QAction::triggered, this, &MailInbox::onForwardMail);
   connect( delete_mail, &QAction::triggered, this, &MailInbox::onDeleteMail);

}

void MailInbox::setupActions()
{
   reply_mail = new QAction( QIcon( ":/images/mail_reply.png"), tr( "Reply"), this );
   reply_all_mail = new QAction( QIcon( ":/images/mail_reply_all.png"), tr( "Reply All"),this );
   forward_mail = new QAction( QIcon( ":/images/mail_forward.png"), tr("Forward"), this);
   delete_mail = new QAction(QIcon( ":/images/delete_icon.png"), tr( "Delete" ), this);
   delete_mail->setShortcut(Qt::Key_Delete);
   //add actions to MailViewer toolbar
   QToolBar* message_tools = ui->current_message->message_tools;
   message_tools->addAction( reply_mail );
   message_tools->addAction( reply_all_mail );
   message_tools->addAction( forward_mail );
   QWidget* spacer = new QWidget(message_tools);
   spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
   message_tools->addWidget(spacer);
   message_tools->addAction( delete_mail );
}

void MailInbox::onReplyMail()
{
   auto msg_window = new MailEditor(this);
   //msg_window->addToContact(contact_id);
   //add quoted text to window
   //set focus to top of window
   msg_window->setFocusAndShow();
}

void MailInbox::onReplyAllMail()
{
   auto msg_window = new MailEditor(this);
   //msg_window->addToContact(contact_id);
   //add quoted text to window
   //set focus to top of window
   msg_window->setFocusAndShow();
}

void MailInbox::onForwardMail()
{
   auto msg_window = new MailEditor(this);
   //msg_window->addToContact(contact_id);
   //add quoted text to window
   //set focus to top of window
   msg_window->setFocusAndShow();
}

void MailInbox::onDeleteMail()
{
   //remove selected mail from inbox model (and database)
   InboxModel* model = static_cast<InboxModel*>(ui->inbox_table->model());
   //ui->inbox_table->setUpdatesEnabled(false);
   QItemSelectionModel* selection_model = ui->inbox_table->selectionModel();
   QModelIndexList indexes = selection_model->selectedRows();
   qSort(indexes);
   for(int i = indexes.count() - 1; i > -1; --i)
       model->removeRows(indexes.at(i).row(),1);
   //ui->inbox_table->setUpdatesEnabled(true);   
}

