#include "Mailbox.hpp"

#include "ui_Mailbox.h"

#include "ATopLevelWindowsContainer.hpp"
#include "FileAttachmentDialog.hpp"
#include "KeyhoteeMainWindow.hpp"
#include "maileditorwindow.hpp"

#include <bts/profile.hpp>
#include <fc/reflect/variant.hpp>

#include <QMessageBox>
#include <QTextEdit>
#include <QToolBar>

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
  return sourceModel()->data(from_index).toString().contains(filterRegExp()) ||
         sourceModel()->data(subject_index).toString().contains(filterRegExp());
  }

void Mailbox::searchEditChanged(QString search_string)
  {
  QSortFilterProxyModel* model = dynamic_cast<QSortFilterProxyModel*>(ui->inbox_table->model());
  QRegExp                regex(search_string, Qt::CaseInsensitive, QRegExp::FixedString);
  model->setFilterRegExp(regex);
  }

Mailbox::Mailbox(ATopLevelWindowsContainer* parent)
  : ui(new Ui::Mailbox() ),
  _type(Inbox),
  _sourceModel(nullptr),
  _mainWindow(parent),
  _attachmentSelected(false)
  {
  ui->setupUi(this);
  setupActions();
  }

void Mailbox::onDoubleClickedItem(QModelIndex index)
  {
  QSortFilterProxyModel* model = dynamic_cast<QSortFilterProxyModel*>(ui->inbox_table->model());
  auto                   sourceModelIndex = model->mapToSource(index);
  auto                   sourceModel = dynamic_cast<MailboxModel*>(model->sourceModel());

  IMailProcessor::TPhysicalMailMessage decodedMsg;
  IMailProcessor::TStoredMailMessage encodedMsg;
  sourceModel->getMessageData(sourceModelIndex, &encodedMsg, &decodedMsg);
  MailEditorMainWindow* mailEditor = new MailEditorMainWindow(_mainWindow,
    sourceModel->getAddressBookModel(), *_mailProcessor, _type == Drafts);
  mailEditor->LoadMessage(this, encodedMsg, decodedMsg, MailEditorMainWindow::TLoadForm::Draft);
  mailEditor->show();
  }

void Mailbox::showCurrentMail(const QModelIndex &selected,
                              const QModelIndex &deselected)
  {}

void Mailbox::checksendmailbuttons()
{
    QItemSelectionModel* selection_model = ui->inbox_table->selectionModel();
    QModelIndexList      indexes = selection_model->selectedRows();

    bool                 oneEmailSelected = (indexes.size() == 1);
    auto app = bts::application::instance();
    auto profile = app->get_profile();

    auto idents = profile->identities();
    reply_mail->setEnabled(oneEmailSelected && (idents.size() > 0));
    reply_all_mail->setEnabled(oneEmailSelected && (idents.size() > 0));
    forward_mail->setEnabled(oneEmailSelected && (idents.size() > 0));
}

void Mailbox::onSelectionChanged(const QItemSelection &selected,
                                 const QItemSelection &deselected)
  {
  QItemSelectionModel* selection_model = ui->inbox_table->selectionModel();
  QModelIndexList      indexes = selection_model->selectedRows();
  //disable reply buttons if more than one email selected
  bool                 oneEmailSelected = (indexes.size() == 1);
  auto app = bts::application::instance();
  auto profile = app->get_profile();

  auto idents = profile->identities();
  reply_mail->setEnabled(oneEmailSelected && (idents.size() > 0));
  reply_all_mail->setEnabled(oneEmailSelected && (idents.size() > 0));
  forward_mail->setEnabled(oneEmailSelected && (idents.size() > 0));

  reply_mail->setEnabled(idents.size() > 0);
  reply_all_mail->setEnabled(idents.size() > 0);
  forward_mail->setEnabled(idents.size() > 0);

  //display selected email(s) in message preview window
  if (oneEmailSelected)
    {
    refreshMessageViewer();
    getKeyhoteeWindow()->setEnabledMailActions(true);
    }
  else
    {
    if (indexes.size() > 1)
      ui->mail_viewer->setCurrentWidget(ui->info_2);
    else
      ui->mail_viewer->setCurrentWidget(ui->info_1);
    //TODO: not implemented ui->current_message->displayMailMessages(indexes,selection_model);

    getKeyhoteeWindow()->setEnabledMailActions(false);
    getKeyhoteeWindow()->setEnabledAttachmentSaveOption( _attachmentSelected = false );
    }

  getKeyhoteeWindow()->setEnabledDeleteOption( indexes.size() > 0 );
  }

Mailbox::~Mailbox()
  {
  delete ui;
  }

void Mailbox::initial(IMailProcessor& mailProcessor, MailboxModel* model, InboxType type,
  ATopLevelWindowsContainer* parentKehoteeMainW)
  {
  _type = type;
  _sourceModel = model;
  _mailProcessor = &mailProcessor;
  _mainWindow = parentKehoteeMainW;

  MailTable::InitialSettings settings;
  readSettings(&settings);

  //enable sorting the mailbox
  QSortFilterProxyModel* proxyModel = new MailSortFilterProxyModel();
  proxyModel->setSourceModel(model);
  ui->inbox_table->setModel(proxyModel);

  QHeaderView *horHeader = ui->inbox_table->horizontalHeader();

  if (_type == Inbox)
    {
    }
  else if (_type == Sent)
    {
    horHeader->swapSections(MailboxModel::To, MailboxModel::From);
    horHeader->swapSections(MailboxModel::DateReceived, MailboxModel::DateSent);
    }
  else if (_type == Drafts)
    {
    horHeader->swapSections(MailboxModel::To, MailboxModel::From);
    horHeader->swapSections(MailboxModel::DateReceived, MailboxModel::DateSent);
    }
  else if (_type == Outbox)
    {
    }  

  
  QList<MailboxModel::Columns> defaultColumns;
  getDefaultColumns(&defaultColumns);
  ui->inbox_table->initial(settings, defaultColumns);

  //connect signals for the new selection model (created by setModel call)
  QItemSelectionModel* inbox_selection_model = ui->inbox_table->selectionModel();
  connect(inbox_selection_model, &QItemSelectionModel::selectionChanged, this, &Mailbox::onSelectionChanged);
  connect(inbox_selection_model, &QItemSelectionModel::currentChanged, this, &Mailbox::showCurrentMail);
  connect(ui->inbox_table, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickedItem(QModelIndex)));

  connect(reply_mail, &QAction::triggered, this, &Mailbox::onReplyMail);
  connect(reply_all_mail, &QAction::triggered, this, &Mailbox::onReplyAllMail);
  connect(forward_mail, &QAction::triggered, this, &Mailbox::onForwardMail);
  connect(delete_mail, &QAction::triggered, this, &Mailbox::onDeleteMail);
  }

void Mailbox::setupActions()
  {
  reply_mail = new QAction(QIcon(":/images/mail_reply.png"), tr("Reply"), this);
  reply_all_mail = new QAction(QIcon(":/images/mail_reply_all.png"), tr("Reply All"), this);
  forward_mail = new QAction(QIcon(":/images/mail_forward.png"), tr("Forward"), this);
  delete_mail = new QAction(QIcon(":/images/delete_icon.png"), tr("Delete"), this);
  //delete_mail->setShortcut(Qt::Key_Delete);
  //add actions to MailViewer toolbar
  QToolBar* message_tools = ui->current_message->message_tools;
  auto app = bts::application::instance();
  auto profile = app->get_profile();

  auto idents = profile->identities();
  if(idents.size() == 0) {
    reply_mail->setEnabled(false);
    reply_all_mail->setEnabled(false);
    forward_mail->setEnabled(false);
  }
  message_tools->addAction(reply_mail);
  message_tools->addAction(reply_all_mail);
  message_tools->addAction(forward_mail);
  QWidget* spacer = new QWidget(message_tools);
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  message_tools->addWidget(spacer);
  message_tools->addAction(delete_mail);
  }

QModelIndex Mailbox::getSelectedMail()
  {
  QItemSelectionModel* selection_model = ui->inbox_table->selectionModel();
  QModelIndexList      sortFilterIndexes = selection_model->selectedRows();
  if (sortFilterIndexes.size() == 1)
    return sortFilterIndexes[0];
  else
    return QModelIndex();
  }

QSortFilterProxyModel* Mailbox::sortedModel()
  {
  return static_cast<QSortFilterProxyModel*>(ui->inbox_table->model());
  }

void Mailbox::duplicateMail(ReplyType replyType)
  {
  IMailProcessor::TPhysicalMailMessage decodedMsg;
  IMailProcessor::TStoredMailMessage encodedMsg;
  if (getSelectedMessageData (&encodedMsg, &decodedMsg) == false)
    return;

  MailEditorMainWindow* mailEditor = new MailEditorMainWindow(_mainWindow,
    _sourceModel->getAddressBookModel(), *_mailProcessor, true);

  MailEditorMainWindow::TLoadForm loadForm = MailEditorMainWindow::TLoadForm::Draft;

  switch(replyType)
    {
    case ReplyType::forward:
      loadForm = MailEditorMainWindow::TLoadForm::Forward;
      break;
    case ReplyType::reply_all:
      loadForm = MailEditorMainWindow::TLoadForm::ReplyAll;
      break;
    case ReplyType::reply:
      loadForm = MailEditorMainWindow::TLoadForm::Reply;
      break;
    default:
      assert(false);
    }

  mailEditor->LoadMessage(this, encodedMsg, decodedMsg, loadForm);
  mailEditor->show();
  }

void Mailbox::onDeleteMail()
  {
  //remove selected mail from inbox model (and database)
  QSortFilterProxyModel* model = dynamic_cast<QSortFilterProxyModel*>(ui->inbox_table->model());
  //model->setUpdatesEnabled(false);
  QItemSelectionModel*   selection_model = ui->inbox_table->selectionModel();
  QModelIndexList        sortFilterIndexes = selection_model->selectedRows();
  if (sortFilterIndexes.empty())
    return;

  if (QMessageBox::question(this, tr("Delete Mail"), tr("Are you sure you want to delete selected email(s)?")) == QMessageBox::Button::No)
    return;
  QModelIndexList indexes;
  for(const QModelIndex& sortFilterIndex : sortFilterIndexes)
    indexes.append(model->mapToSource(sortFilterIndex));
  qSort(indexes);
  auto sourceModel = model->sourceModel();
  for (int i = indexes.count() - 1; i > -1; --i)
    sourceModel->removeRows(indexes.at(i).row(), 1);
  //model->setUpdatesEnabled(true);
  qSort(sortFilterIndexes);
  selectNextRow(sortFilterIndexes.takeLast().row(), indexes.count());
  }

bool Mailbox::isShowDetailsHidden()
  {
  return ui->mail_viewer->isHidden();
  }

void Mailbox::refreshMessageViewer()
  {
  QItemSelectionModel* selection_model = ui->inbox_table->selectionModel();
  QModelIndexList      indexes = selection_model->selectedRows();
  //disable reply buttons if more than one email selected
  bool                 oneEmailSelected = (indexes.size() == 1);
  //display selected email(s) in message preview window
  if (oneEmailSelected)
    {
    QSortFilterProxyModel* model = dynamic_cast<QSortFilterProxyModel*>(ui->inbox_table->model());
    QModelIndex sourceModelIndex = model->mapToSource(indexes[0]);
    MailboxModel* sourceModel = dynamic_cast<MailboxModel*>(model->sourceModel());
    ui->mail_viewer->setCurrentWidget(ui->current_message);
    ui->current_message->displayMailMessage(this, sourceModelIndex, sourceModel);
    _attachmentSelected = sourceModel->hasAttachments(sourceModelIndex);
    getKeyhoteeWindow()->setEnabledAttachmentSaveOption(_attachmentSelected);
    }
  }

void Mailbox::removeMessage(const IMailProcessor::TStoredMailMessage& msg)
  {
  QSortFilterProxyModel* model = dynamic_cast<QSortFilterProxyModel*>(ui->inbox_table->model());

  QModelIndex foundModelIndex = _sourceModel->findModelIndex(msg);
  if(foundModelIndex.isValid())
    {
    QModelIndex proxyModelIndex = model->mapFromSource(foundModelIndex);
    assert(proxyModelIndex.isValid());
    model->removeRow(proxyModelIndex.row());
    }
  }

void Mailbox::on_actionShow_details_toggled(bool checked)
  {
  if (checked)
    ui->mail_viewer->show();
  else
    ui->mail_viewer->hide();

  }

bool Mailbox::isAttachmentSelected () const
  {
  return _attachmentSelected;
  }

bool Mailbox::isSelection () const
  {
  QItemSelectionModel* selection_model = ui->inbox_table->selectionModel();
  QModelIndexList      indexes = selection_model->selectedRows();
  return (indexes.size() > 0);
  }

bool Mailbox::isOneEmailSelected() const
  {
  QItemSelectionModel* selection_model = ui->inbox_table->selectionModel();
  QModelIndexList      indexes = selection_model->selectedRows();
  return (indexes.size() == 1);
  }

void Mailbox::saveAttachment ()
{
  IMailProcessor::TPhysicalMailMessage decodedMsg;
  IMailProcessor::TStoredMailMessage encodedMsg;
  if (getSelectedMessageData (&encodedMsg, &decodedMsg) == false)
    return;

  FileAttachmentDialog* attachmentsDlg = new FileAttachmentDialog(this);
  attachmentsDlg->setModal(true);
  attachmentsDlg->loadAttachments(decodedMsg);
  if (decodedMsg.attachments.size () == 1)
  {
    attachmentsDlg->saveAttachments ();
  }
  else
  {
    attachmentsDlg->exec ();
  }
  delete attachmentsDlg;
}

void Mailbox::selectNextRow(int idx, int deletedRowCount) const
{
  if (_sourceModel->rowCount() == 0)
    return;
  int nextIdx = idx + 1 - deletedRowCount;
  if (nextIdx < _sourceModel->rowCount())
    ui->inbox_table->selectRow(nextIdx);
  else
    ui->inbox_table->selectRow(_sourceModel->rowCount() - 1);
}

bool Mailbox::getSelectedMessageData (IMailProcessor::TStoredMailMessage* encodedMsg, 
                             IMailProcessor::TPhysicalMailMessage* decodedMsg)
{
  QModelIndex index = getSelectedMail();
  if(index.isValid() == false)
    return false;

  QSortFilterProxyModel* model = dynamic_cast<QSortFilterProxyModel*>(ui->inbox_table->model());
  QModelIndex sourceModelIndex = model->mapToSource(index);
  _sourceModel->getMessageData(sourceModelIndex, encodedMsg, decodedMsg);

  return true;
}
    
void Mailbox::selectAll ()
{
  ui->inbox_table->selectAll();
  ui->inbox_table->setFocus();
}

void Mailbox::writeSettings()
{
  QSettings settings("Invictus Innovations", "Keyhotee");

  settings.beginGroup("MailBox");
  //MailBox type
  settings.beginGroup( QString::number(_type) );

  settings.setValue("SortColumn", ui->inbox_table->horizontalHeader()->sortIndicatorSection());
  settings.setValue("SortOrder", ui->inbox_table->horizontalHeader()->sortIndicatorOrder());
  settings.setValue("SelectedColumns", encodeSelectedColumns());
  
  settings.endGroup();
  settings.endGroup();
}

void Mailbox::readSettings(MailTable::InitialSettings* initSettings)
{
  QSettings settings("Invictus Innovations", "Keyhotee");

  settings.beginGroup("MailBox");
  //MailBox type
  settings.beginGroup( QString::number(_type) );  

  initSettings->sortColumn = settings.value("SortColumn", QVariant(0)).toInt();
  initSettings->sortOrder = static_cast<Qt::SortOrder>( settings.value("SortOrder", QVariant(0)).toInt() );

  unsigned int columnsEncode = settings.value("SelectedColumns", QVariant(0)).toUInt();
  decodeSelectedColumns(columnsEncode, &initSettings->columns);

  settings.endGroup();
  settings.endGroup();
}

unsigned int Mailbox::encodeSelectedColumns()
{
  QHeaderView* header = ui->inbox_table->horizontalHeader();
  unsigned int columnsCount = header->count(); 
  unsigned int selectedColumns = 0x0000;

  //max columns = 32, 
  //each column encode to one bit  
  assert (selectedColumns * 8/*bits*/ < columnsCount);
  for (unsigned int i = 0; i < columnsCount; ++i)
  {
    // physical index convsrt to logical index
    int columnType = header->logicalIndex (i/*visualIndex*/);
    if (!header->isSectionHidden(columnType) )
    {
      selectedColumns |= 1 << columnType;
    }
  }

  return selectedColumns;
}

void Mailbox::decodeSelectedColumns(unsigned int columnsEncode, 
                             QList<MailboxModel::Columns>* columnsDecode)
{

  for (unsigned int i = 0; i < MailboxModel::NumColumns; ++i)
  {
    if ( (1 << i) & columnsEncode )
      columnsDecode->push_back(static_cast<MailboxModel::Columns>(i));
  }
}

void Mailbox::getDefaultColumns(QList<MailboxModel::Columns>* defaultColumns)
{
  defaultColumns->push_back(MailboxModel::Read);
  //defaultColumns->push_back(MailboxModel::Money);
  defaultColumns->push_back(MailboxModel::Attachment);
  //defaultColumns->push_back(MailboxModel::Reply);
  //defaultColumns->push_back(MailboxModel::Chat);
  defaultColumns->push_back(MailboxModel::From);
  defaultColumns->push_back(MailboxModel::Subject);
  //defaultColumns->push_back(MailboxModel::DateReceived);
  defaultColumns->push_back(MailboxModel::To);
  defaultColumns->push_back(MailboxModel::DateSent);
  //defaultColumns->push_back(MailboxModel::Status);
                                          
  if (_type == Inbox)                     
  {
  }
  else if (_type == Sent)
  {
    defaultColumns->push_back(MailboxModel::Status);
  }
  else if (_type == Drafts)
  {
  }
  else if (_type == Outbox)
  {
    defaultColumns->push_back(MailboxModel::DateReceived);
    defaultColumns->push_back(MailboxModel::Status);
  }
}