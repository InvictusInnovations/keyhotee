#include "Mailbox.hpp"

#include "ui_Mailbox.h"

#include "ATopLevelWindowsContainer.hpp"
#include "FileAttachmentDialog.hpp"
#include "KeyhoteeMainWindow.hpp"
#include "maileditorwindow.hpp"

#include "../qtreusable/HeaderWidget.hpp"

#include <bts/profile.hpp>
#include <fc/reflect/variant.hpp>

#include <QMessageBox>
#include <QMetaEnum>
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

void Mailbox::onOpenMail()
{
  QItemSelectionModel* selection_model = ui->inbox_table->selectionModel();
  QModelIndexList      indexes = selection_model->selectedRows();
  onDoubleClickedItem(indexes.first());
}
void Mailbox::onMarkAsUnreadMail()
{
  QItemSelectionModel* selection_model = ui->inbox_table->selectionModel();
  QModelIndexList      indexes = selection_model->selectedRows();
  QSortFilterProxyModel* model = dynamic_cast<QSortFilterProxyModel*>(ui->inbox_table->model());
  MailboxModel* sourceModel = dynamic_cast<MailboxModel*>(model->sourceModel());
  foreach(QModelIndex index, indexes)
  {
    QModelIndex mapped_index = model->mapToSource(index);
    sourceModel->markMessageAsUnread(mapped_index);
  }
}
void Mailbox::showCurrentMail(const QModelIndex &selected,
                              const QModelIndex &deselected)
  {}

void Mailbox::checkSendMailButtons()
{
    QItemSelectionModel* selection_model = ui->inbox_table->selectionModel();
    QModelIndexList      indexes = selection_model->selectedRows();

    bool  oneEmailSelected = (indexes.size() == 1);
    bool  identity = isIdentity();

    ui->actionReply->setEnabled(oneEmailSelected && identity);
    ui->actionReply_All->setEnabled(oneEmailSelected && identity);
    ui->actionForward->setEnabled(oneEmailSelected && identity);

    getKeyhoteeWindow()->onEnableMailButtons(oneEmailSelected && identity);
}

void Mailbox::onSelectionChanged(const QItemSelection &selected,
                                 const QItemSelection &deselected)
  {
  QItemSelectionModel* selection_model = ui->inbox_table->selectionModel();
  QModelIndexList      indexes = selection_model->selectedRows();
  //disable reply buttons if more than one email selected
  bool                 oneEmailSelected = (indexes.size() == 1);

  ui->actionOpen->setEnabled(oneEmailSelected);
  ui->actionMark_as_unread->setEnabled(indexes.size() > 0);
  ui->actionDelete->setEnabled(indexes.size() > 0);

  checkSendMailButtons();

  //display selected email(s) in message preview window
  if (oneEmailSelected)
    {
    refreshMessageViewer();    
    }
  else
    {
    if (indexes.size() > 1)
      ui->mail_viewer->setCurrentWidget(ui->info_2);
    else
      ui->mail_viewer->setCurrentWidget(ui->info_1);
    //TODO: not implemented ui->current_message->displayMailMessages(indexes,selection_model);

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

  /// convert enum InboxType to string
  const QMetaObject &metaObject = Mailbox::staticMetaObject;
  QMetaEnum metaEnum = metaObject.enumerator(0);
  ui->header->initial(tr(metaEnum.key(_type)));

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
  else if(_type == Spam)
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

  connect(ui->actionOpen, &QAction::triggered, this, &Mailbox::onOpenMail);
  connect(ui->actionMark_as_unread, &QAction::triggered, this, &Mailbox::onMarkAsUnreadMail);
  connect(ui->actionDelete, &QAction::triggered, this, &Mailbox::onDeleteMail);
  connect(ui->actionReply, &QAction::triggered, this, &Mailbox::onReplyMail);
  connect(ui->actionReply_All, &QAction::triggered, this, &Mailbox::onReplyAllMail);
  connect(ui->actionForward, &QAction::triggered, this, &Mailbox::onForwardMail);
  }

void Mailbox::setupActions()
  {
  /// Add actions to MailViewer toolbar
  QToolBar* message_tools = ui->current_message->message_tools;
  auto app = bts::application::instance();
  auto profile = app->get_profile();

  message_tools->addAction(ui->actionReply);
  message_tools->addAction(ui->actionReply_All);
  message_tools->addAction(ui->actionForward);
  QWidget* spacer = new QWidget(message_tools);
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  message_tools->addWidget(spacer);
  message_tools->addAction(ui->actionDelete);

  QAction* separator1 = new QAction(this);
  separator1->setSeparator(true);
  QAction* separator2 = new QAction(this);
  separator2->setSeparator(true);

  ui->inbox_table->addAction(ui->actionOpen);
  ui->inbox_table->addAction(ui->actionMark_as_unread);
  ui->inbox_table->addAction(separator1);
  ui->inbox_table->addAction(ui->actionDelete);
  ui->inbox_table->addAction(separator2);
  ui->inbox_table->addAction(ui->actionReply);
  ui->inbox_table->addAction(ui->actionReply_All);
  ui->inbox_table->addAction(ui->actionForward);

  ui->actionOpen->setEnabled(false);
  ui->actionMark_as_unread->setEnabled(false);
  ui->actionDelete->setEnabled(false);
  ui->actionReply->setEnabled(false);
  ui->actionReply_All->setEnabled(false);
  ui->actionForward->setEnabled(false);
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
  else if(_type == Spam)
  {
  }
}
bool Mailbox::isIdentity()
{
  auto app = bts::application::instance();
  auto profile = app->get_profile();
  auto idents = profile->identities();
  if (idents.empty())
    return false;
  else
    return true;
}
