#include "fileattachmentwidget.hpp"

#include "ui_fileattachmentwidget.h"

#include <QFileDialog>

static const char* fileSizeUnit[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB", "BB"};

const unsigned int UNIT_MAX_IDX = sizeof(fileSizeUnit)/sizeof(const char*);

const int NAME_COLUMN_IDX = 0;
const int SIZE_COLUMN_IDX = 1;

/** Represents attachment table widget item holding additional data needed to simplify
    operations performed over table items.
*/
class TFileAttachmentWidget::TFileAttachmentItem : public QTableWidgetItem
  {
  public:
    /// Constructor to build item representing file name cell.
    TFileAttachmentItem(const QFileInfo& fileInfo, TFileAttachmentWidget* owner) :
      QTableWidgetItem(fileInfo.fileName()),
      FileInfo(fileInfo),
      Owner(owner),
      FileNameInfo(nullptr)
      {
      assert(owner != nullptr);
      
      AttachmentListPos = owner->AttachmentList.insert(owner->AttachmentList.end(), this);
      owner->TotalAttachmentSize += fileInfo.size();

      setToolTip(fileInfo.absoluteFilePath());
      }

    TFileAttachmentItem(TFileAttachmentItem* fileNameInfo, const TScaledSize& scaledSize) :
      FileInfo(fileNameInfo->FileInfo),
      Owner(nullptr),
      FileNameInfo(fileNameInfo)
      {
      QString text = TFileAttachmentWidget::toString(scaledSize);
      setText(text);
      }

    virtual ~TFileAttachmentItem() {}

    /** Retrieves file name being displayed. It can be different than real underlying file name when
        user rename the attachment.
    */
    QString GetDisplayedFileName() const
      {
      if(FileNameInfo != nullptr)
        return FileNameInfo->GetDisplayedFileName();

      return text();
      }

    /// Returns associated physical file info.
    const QFileInfo& GetFileInfo() const
      {
      return FileInfo;
      }

    /// Unregisters item from file attachment list.
    void Unregister()
      {
      if(FileNameInfo != nullptr)
        {
        FileNameInfo->Unregister();
        FileNameInfo = nullptr;
        }
      else
      if(Owner != nullptr)
        {
        Owner->AttachmentList.erase(AttachmentListPos);

        TFileInfoList::iterator foundPos = std::find(Owner->AttachmentIndex.begin(),
          Owner->AttachmentIndex.end(), FileInfo);

        Owner->AttachmentIndex.erase(foundPos);
        Owner->TotalAttachmentSize -= FileInfo.size();

        AttachmentListPos = TFileAttachmentListPos();
        /// To avoid multiple unregistrations.
        Owner = nullptr;
        }
      }

  /// QTableWidgetItem override.
    virtual TFileAttachmentItem* clone() const override
      {
      TFileAttachmentItem* cloned = new TFileAttachmentItem(*this);
      
      /// Cloned item should not be unregistered from containers.
      cloned->FileNameInfo = nullptr;
      cloned->AttachmentListPos = TFileAttachmentListPos();
      
      return cloned;
      }

  /// Class attributes:
  private:
    typedef TFileAttachmentList::iterator TFileAttachmentListPos;

    QFileInfo               FileInfo;
    TFileAttachmentWidget*  Owner;
    /// Non-null when this object represents a 'size' item.
    TFileAttachmentItem*    FileNameInfo;
    /// Filled in when item is attached to the list.
    TFileAttachmentListPos  AttachmentListPos;
  };

TFileAttachmentWidget::TFileAttachmentWidget(QWidget *parent, bool editMode) :
  QWidget(parent),
  ui(new Ui::TFileAttachmentWidget),
  TotalAttachmentSize(0),
  EditMode(editMode)
  {
  ui->setupUi(this);

  ui->addButton->setDefaultAction(ui->actionAdd);
  ui->delButton->setDefaultAction(ui->actionDel);
  ui->saveButton->setDefaultAction(ui->actionSave);

  /// Call it to validate buttons state at startup
  onAttachementTableSelectionChanged();

  ConfigureContextMenu();
  ConfigureAttachmentTable();
  }

TFileAttachmentWidget::~TFileAttachmentWidget()
  {
  delete ui;
  }

void TFileAttachmentWidget::LoadAttachedFiles(const TAttachmentContainer& attachedFiles)
  {
  }

bool TFileAttachmentWidget::GetAttachedFiles(TAttachmentContainer* storage,
  TFileInfoList* failedFilesStorage) const
  {
  assert(storage != nullptr);
  assert(failedFilesStorage != nullptr);

  storage->reserve(AttachmentList.size());

  for(const auto& item : AttachmentList)
    AttachFile(*item, storage, failedFilesStorage);

  return failedFilesStorage->empty();
  }

void TFileAttachmentWidget::ConfigureContextMenu()
  {
  /** Register all actions to be displayed in ctx menu in a table widget (it should have configured
      ContextMenuPolicy to ActionsContextMenu).
  */
  ui->attachmentTable->addAction(ui->actionAdd);
  ui->attachmentTable->addAction(ui->actionDel);
  ui->attachmentTable->addAction(ui->actionSave);
  ui->attachmentTable->addAction(ui->actionRename);
  QAction* sep = new QAction(this);
  sep->setSeparator(true);
  ui->attachmentTable->addAction(sep);
  ui->attachmentTable->addAction(ui->actionSelectAll);
  }

void TFileAttachmentWidget::ConfigureAttachmentTable()
  {
  QSize tableSize = ui->attachmentTable->size();
  unsigned int width = tableSize.width()/3;
  width *= 2;
  ui->attachmentTable->setColumnWidth(NAME_COLUMN_IDX, width);
  width = tableSize.width() - width;
  ui->attachmentTable->setColumnWidth(SIZE_COLUMN_IDX, width);

  UpdateColumnHeaders();
  }

void TFileAttachmentWidget::UpdateColumnHeaders()
  {
  QString text = QString::number(AttachmentList.size()) + " attachment(s)";
  QTableWidgetItem* header = ui->attachmentTable->horizontalHeaderItem(NAME_COLUMN_IDX);
  header->setText(text);

  TScaledSize scaledSize = ScaleAttachmentSize(TotalAttachmentSize);

  text = toString(scaledSize);
  header = ui->attachmentTable->horizontalHeaderItem(SIZE_COLUMN_IDX);
  header->setText(text);
  }

inline
TFileAttachmentWidget::TScaledSize 
TFileAttachmentWidget::ScaleAttachmentSize(unsigned long long size) const
  {
  unsigned int unitIdx = 0;

  double fSize = (double)size;
  while(fSize > 1024.0)
    {
    fSize /= 1024.0;
    if(unitIdx == UNIT_MAX_IDX - 1)
      break;

    ++unitIdx;
    }

  return TScaledSize(fSize, fileSizeUnit[unitIdx]);
  }

inline
bool TFileAttachmentWidget::FreezeAttachmentTable()
  {
  ui->attachmentTable->setDisabled(true);
  bool sortEnabled = ui->attachmentTable->isSortingEnabled();
  ui->attachmentTable->setSortingEnabled(false);

  return sortEnabled;
  }

inline
void TFileAttachmentWidget::UnFreezeAttachmentTable(bool sortEnabled)
  {
  ui->attachmentTable->setSortingEnabled(sortEnabled);
  ui->attachmentTable->setDisabled(false);
  }

void TFileAttachmentWidget::AttachFile(const TFileAttachmentItem& item, TAttachmentContainer* storage,
  TFileInfoList* failedFilesStorage) const
  {
  const QFileInfo& fileInfo = item.GetFileInfo();

  QFile file(fileInfo.absoluteFilePath());
  bool success = file.open(QIODevice::ReadOnly);
  if(success)
    {
    storage->push_back(TPhysicalAttachment());
    TPhysicalAttachment& physicalAttachment = storage->back();

    physicalAttachment.filename = item.GetDisplayedFileName().toStdString();
    qint64 fileSize = file.size();
    physicalAttachment.body.resize(fileSize);
    qint64 readLen = file.read(physicalAttachment.body.data(), fileSize);
    file.close();

    if(readLen != fileSize)
      {
      /// File read was incomplete. Mark it as failure and remove from physical attachment.
      storage->pop_back();
      failedFilesStorage->push_back(fileInfo);
      }
    }
  else
    {
    failedFilesStorage->push_back(fileInfo);
    }
  }

QString TFileAttachmentWidget::toString(const TScaledSize& scaledSize)
  {
  return QString::number(scaledSize.first, 'f', 1) + tr(" ") + QString(scaledSize.second);
  }

void TFileAttachmentWidget::onAddTriggered()
  {
  bool sortEnabled = FreezeAttachmentTable();

  QStringList selectedFiles = QFileDialog::getOpenFileNames(this, "File(s) to attach", QString("."));
  
  for(QStringList::const_iterator fileNameIt = selectedFiles.constBegin();
      fileNameIt != selectedFiles.constEnd(); ++fileNameIt)
    {
    const QString& fileName = *fileNameIt;

    ///probably it will be wrong strip unicode chars: .toLocal8Bit().constData());
    QFileInfo fileInfo(fileName);
    if(fileInfo.exists() == false)
      continue;

    /** Ignore redundant files (it was originally implemented this way) but others mail clients
        allow this.
    */
    TFileInfoList::const_iterator foundPos = std::find(AttachmentIndex.begin(),
      AttachmentIndex.end(), fileInfo);
    if(foundPos != AttachmentIndex.end())
      continue;
    
    AttachmentIndex.push_back(fileInfo);

    unsigned long long size = fileInfo.size();
    TScaledSize scaledSize = ScaleAttachmentSize(size);

    /// Allocate objects representing table items - name item automatically will register in the list.
    TFileAttachmentItem* fileNameItem = new TFileAttachmentItem(fileInfo, this);
    TFileAttachmentItem* fileSizeItem = new TFileAttachmentItem(fileNameItem, scaledSize);
    
    unsigned int row = AttachmentList.size();

    ui->attachmentTable->setRowCount(row);
    ui->attachmentTable->setItem(row-1, NAME_COLUMN_IDX, fileNameItem);
    ui->attachmentTable->setItem(row-1, SIZE_COLUMN_IDX, fileSizeItem);
    }

  UnFreezeAttachmentTable(sortEnabled);
  
  UpdateColumnHeaders();
  }

void TFileAttachmentWidget::onDelTriggered()
  {
  QList<QTableWidgetItem*> selection = ui->attachmentTable->selectedItems();

  bool sortEnabled = FreezeAttachmentTable();

  for(auto itemIt = selection.cbegin(); itemIt != selection.cend(); ++itemIt)
    {
    QTableWidgetItem* item = *itemIt;
    assert(dynamic_cast<TFileAttachmentItem*>(item) != nullptr);
    TFileAttachmentItem* fileItem = static_cast<TFileAttachmentItem*>(item);
    fileItem->Unregister();
    int rowNo = fileItem->row();
    ui->attachmentTable->removeRow(rowNo);
    }

  UnFreezeAttachmentTable(sortEnabled);

  UpdateColumnHeaders();
  }

void TFileAttachmentWidget::onSaveTriggered()
  {
  }

void TFileAttachmentWidget::onAttachementTableSelectionChanged()
  {
  auto selectedItems = ui->attachmentTable->selectedItems();

  unsigned int selectedCount = selectedItems.size();
  bool anySelection = selectedCount != 0;
  bool singleSelection = selectedCount == 1;

  ui->actionDel->setEnabled(anySelection && EditMode);
  ui->actionSave->setEnabled(anySelection && (EditMode == false));
  ui->actionRename->setEnabled(singleSelection && EditMode);
  }

