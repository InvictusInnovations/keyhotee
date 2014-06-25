#include "MailViewer.hpp"

#include "ui_MailViewer.h"

#include "KeyhoteeMainWindow.hpp"
#include "MailboxModel.hpp"
#include "Mailbox.hpp"
#include "utils.hpp"

#include <QImageReader>
#include <QToolBar>

MailViewer::MailViewer(QWidget* parent)
: /*QWidget(parent),*/ ui( new Ui::MailViewer() )
  {
  ui->setupUi(this);
  message_tools = new QToolBar(ui->toolbar_container);
  QGridLayout* grid_layout = new QGridLayout(ui->toolbar_container);
  grid_layout->setContentsMargins(0, 0, 0, 0);
  grid_layout->setSpacing(0);
  ui->toolbar_container->setLayout(grid_layout);
  grid_layout->addWidget(message_tools, 0, 0);  
  ui->message_content->setOpenExternalLinks (true);
  ui->message_content->setReadOnly(true);
  ui->remoteContentAlert->initial(this);
  ui->message_content->initial(this);
  }

MailViewer::~MailViewer()
  {
  delete ui;
  }

void MailViewer::displayMailMessage(Mailbox* mailbox, const QModelIndex& index, MailboxModel* mailboxModel)
  {
  assert(index.isValid());
  assert(mailboxModel != nullptr);

  //TODO: later, possibly set a timer and only mark as read if still displaying
  //      this message when timer expires?
  mailboxModel->markMessageAsRead(index);
  MessageHeader msg;
  mailboxModel->getFullMessage(index, msg);
  QString       formatted_date = msg.date_sent.toString(Qt::DefaultLocaleShortDate);
  ui->date_label->setText(formatted_date);
  ui->from_label->clear();
  std::vector<fc::ecc::public_key> sender;
  sender.push_back(msg.header.from_key);
  ui->from_label->SetCollectedContacts(sender);
  ui->from_label->setStyleSheet("QTextEdit { background-color: rgb(255, 255, 255, 0) }");
  if (msg.to_list.size())
    {
    ui->to_prefix->show();
    ui->to_label->show();
    ui->to_label->clear();
    ui->to_label->SetCollectedContacts(msg.to_list);
    ui->to_label->setStyleSheet("QTextEdit { background-color: rgb(255, 255, 255, 0) }");
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
    ui->cc_label->clear();
    ui->cc_label->SetCollectedContacts(msg.cc_list);
    ui->cc_label->setStyleSheet("QTextEdit { background-color: rgb(255, 255, 255, 0) }");
    }
  else
    {
    ui->cc_prefix->hide();
    ui->cc_label->hide();
    }
  //TODO: add to and cc lists
  ui->subject_label->setText(msg.subject);

  /// Hide alert about blocking images
  ui->remoteContentAlert->hide();
  /// notification about blocking images (onBlockedImage) can come for few second after call 'loadContents'
  ui->message_content->loadContents(msg.body, msg.attachments);

  /// Scroll mail window to the top
  ui->message_content->moveCursor (QTextCursor::Start);  
  }

void MailViewer::onBlockedImage()
{
  /// Show alert if any remote image is blocked
  ui->remoteContentAlert->show();
}

void MailViewer::onLoadBlockedImages()
{
  /// Hide alert about blocking images
  ui->remoteContentAlert->hide();
  ui->message_content->loadBlockedImages();
}