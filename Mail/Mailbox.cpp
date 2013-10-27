#include "Mailbox.hpp"
#include "../ui_Mailbox.h"
#include "MailboxModel.hpp"
#include <fc/reflect/variant.hpp>
#include "MailEditor.hpp"
#include <QToolBar>
#include <bts/profile.hpp>


class MailSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    MailSortFilterProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent) {}
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

bool MailSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex from_index = sourceModel()->index(sourceRow, MailboxModel::From, sourceParent);
    QModelIndex subject_index = sourceModel()->index(sourceRow, MailboxModel::Subject, sourceParent);
    return (sourceModel()->data(from_index).toString().contains(filterRegExp()) ||
            sourceModel()->data(subject_index).toString().contains(filterRegExp()));
}

void Mailbox::searchEditChanged(QString search_string)
{
   QSortFilterProxyModel* model = dynamic_cast<QSortFilterProxyModel*>(ui->inbox_table->model());
   QRegExp regex(search_string, Qt::CaseInsensitive, QRegExp::FixedString);
   model->setFilterRegExp(regex);
}

Mailbox::Mailbox( QWidget* parent )
: ui( new Ui::Mailbox() ),
  _type(Inbox),
  _sourceModel(nullptr)
{
   ui->setupUi( this );
   setupActions();
}


void Mailbox::showCurrentMail(const QModelIndex &selected,
                                 const QModelIndex &deselected)
{
}

void Mailbox::onSelectionChanged(const QItemSelection &selected,
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
   QSortFilterProxyModel* model = dynamic_cast<QSortFilterProxyModel*>(ui->inbox_table->model());
   foreach (QModelIndex index, items) 
   {
      auto sourceModelIndex = model->mapToSource(index);
      _sourceModel->getFullMessage(sourceModelIndex,message_header);
      msgs.push_back(message_header);
   }
   ui->current_message->displayMailMessages(msgs);
}

Mailbox::~Mailbox()
{
}

void Mailbox::setModel( MailboxModel* model, InboxType type )
{
   _type = type;
   _sourceModel = model;
   //enable sorting the mailbox
   QSortFilterProxyModel* proxyModel = new MailSortFilterProxyModel();
   proxyModel->setSourceModel( model );
   ui->inbox_table->setModel( proxyModel ); 
   //ui->inbox_table->sortByColumn(0, Qt::AscendingOrder);
   //ui->inbox_table->setModel( model ); 

   ui->inbox_table->horizontalHeader()->resizeSection( MailboxModel::To, 120 );
   ui->inbox_table->horizontalHeader()->resizeSection( MailboxModel::Subject, 300 );
   ui->inbox_table->horizontalHeader()->resizeSection( MailboxModel::DateReceived, 120 );
   ui->inbox_table->horizontalHeader()->resizeSection( MailboxModel::From, 120 );
   ui->inbox_table->horizontalHeader()->resizeSection( MailboxModel::DateSent, 120 );
   if( _type == Inbox )
   {
      ui->inbox_table->horizontalHeader()->hideSection( MailboxModel::Status );
      ui->inbox_table->horizontalHeader()->hideSection( MailboxModel::DateSent );
   }
   if( _type == Sent )
   {
      ui->inbox_table->horizontalHeader()->swapSections( MailboxModel::To, MailboxModel::From );
      ui->inbox_table->horizontalHeader()->swapSections( MailboxModel::DateReceived, MailboxModel::DateSent );
      ui->inbox_table->horizontalHeader()->hideSection( MailboxModel::DateReceived );
   }
   if( _type == Drafts )
   {
      ui->inbox_table->horizontalHeader()->swapSections( MailboxModel::To, MailboxModel::From );
      ui->inbox_table->horizontalHeader()->swapSections( MailboxModel::DateReceived, MailboxModel::DateSent );
      ui->inbox_table->horizontalHeader()->hideSection( MailboxModel::DateReceived );
      ui->inbox_table->horizontalHeader()->hideSection( MailboxModel::Status );
   }

   ui->inbox_table->horizontalHeader()->setSectionsMovable(true);
   ui->inbox_table->horizontalHeader()->setSortIndicatorShown(false);
   ui->inbox_table->horizontalHeader()->setSectionsClickable(true);
   ui->inbox_table->horizontalHeader()->setHighlightSections(true);

   //connect signals for the new selection model (created by setModel call)
   QItemSelectionModel* inbox_selection_model = ui->inbox_table->selectionModel();
   connect( inbox_selection_model, &QItemSelectionModel::selectionChanged, this, &Mailbox::onSelectionChanged );
   connect( inbox_selection_model, &QItemSelectionModel::currentChanged, this, &Mailbox::showCurrentMail );

   connect( reply_mail, &QAction::triggered, this, &Mailbox::onReplyMail);
   connect( reply_all_mail, &QAction::triggered, this, &Mailbox::onReplyAllMail);
   connect( forward_mail, &QAction::triggered, this, &Mailbox::onForwardMail);
   connect( delete_mail, &QAction::triggered, this, &Mailbox::onDeleteMail);

}

void Mailbox::setupActions()
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

QModelIndex Mailbox::getSelectedMail()
{
   QItemSelectionModel* selection_model = ui->inbox_table->selectionModel();
   QModelIndexList sortFilterIndexes = selection_model->selectedRows();
   if (sortFilterIndexes.size() == 1)
      return sortFilterIndexes[0];
   else
      return QModelIndex();
}

QSortFilterProxyModel* Mailbox::sortedModel()
{
   return  static_cast<QSortFilterProxyModel*>(ui->inbox_table->model());
}


void Mailbox::replyMail(bool reply_all)
{
   QModelIndex index = getSelectedMail();
   if (index == QModelIndex())
     return;
   MessageHeader header;
   QModelIndex mappedIndex = sortedModel()->mapToSource(index);
   _sourceModel->getFullMessage(mappedIndex,header);
   auto msg_window = new MailEditor(this);
   auto addressbook = bts::get_profile()->get_addressbook();
   auto reply_to_contact = addressbook->get_contact_by_public_key( header.header.from_key );
   if (reply_to_contact)
      msg_window->addToContact(reply_to_contact->wallet_index);
   //TODO else we need way to show raw public key

   if (reply_all)
   {
   //TODO add check to avoid replying to self as well
      foreach(auto to_key,header.to_list)
      {
         auto to_contact = addressbook->get_contact_by_public_key( to_key );
         if (to_contact)
            msg_window->addToContact(to_contact->wallet_index);            
      }
      foreach(auto cc_key, header.cc_list)
      {
         auto cc_contact = addressbook->get_contact_by_public_key( cc_key );
         if (cc_contact)
            msg_window->addCcContact(cc_contact->wallet_index);            
      }
   }
   //add quoted text to window
   //set focus to top of window
   msg_window->setFocusAndShow();
}

void Mailbox::onForwardMail()
{
   auto msg_window = new MailEditor(this);
   //msg_window->addToContact(contact_id);
   //add quoted text to window
   //set focus to top of window
   msg_window->setFocusAndShow();
}

void Mailbox::onDeleteMail()
{
   //remove selected mail from inbox model (and database)
   QSortFilterProxyModel* model = dynamic_cast<QSortFilterProxyModel*>(ui->inbox_table->model());
   //model->setUpdatesEnabled(false);
   QItemSelectionModel* selection_model = ui->inbox_table->selectionModel();
   QModelIndexList sortFilterIndexes = selection_model->selectedRows();
   QModelIndexList indexes;
   foreach(QModelIndex sortFilterIndex,sortFilterIndexes)
     indexes.append(model->mapToSource(sortFilterIndex));
   qSort(indexes);
   auto sourceModel = model->sourceModel();
   for(int i = indexes.count() - 1; i > -1; --i)
       sourceModel->removeRows(indexes.at(i).row(),1);
   //model->setUpdatesEnabled(true);   
}

