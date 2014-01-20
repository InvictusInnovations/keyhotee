#include "fileattachmentwidget.hpp"

#include "ui_fileattachmentwidget.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QUrl>

static const char* fileSizeUnit[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB", "BB"};

const unsigned int UNIT_MAX_IDX = sizeof(fileSizeUnit)/sizeof(const char*);

const int NAME_COLUMN_IDX = 0;
const int SIZE_COLUMN_IDX = 1;

/** Base class for all representations of attachment items.
    Defines its base functionality, and holds common data.
*/
class TFileAttachmentWidget::AAttachmentItem : public QTableWidgetItem
  {
  public:
    /** Needed to unregister item instance from all helper containers it was registered to.
    */
    virtual void Unregister()
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

        AttachmentListPos = TFileAttachmentListPos();
        /// To avoid multiple unregistrations.
        Owner = nullptr;
        }
      }

    /** Allows to open attachment item using default application registerd for given document type
        in the OS.
    */
    void Open() const
      {
      const QString title(tr("Open attachment..."));
      TFileAttachmentWidget* owner = GetOwner();

      QString fileName(GetDisplayedFileName());
      /// FIXME - actually each created temp file should be registered in the main app to be deleted at exit.
      QTemporaryFile tempFile(QDir::tempPath() + "/XXXXXX." + fileName);
      tempFile.setAutoRemove(false);
      if(tempFile.open() == false)
        {
        QMessageBox::warning(owner, title, tr("Cannot open temporary file to store attachment: ") +
            fileName);
        return;
        }

      QFileInfo tfi(tempFile);

      if(owner->SaveAttachmentItem(this, tfi, false))
        {
        QUrl url(QUrl::fromLocalFile(tfi.absoluteFilePath()));
        if(QDesktopServices::openUrl(url) == false)
          {
          /// Report a message if opening failed.
          QMessageBox::warning(owner, title, tr("Cannot open attachment using its default application: ") +
             fileName);
          }
        }

      tempFile.close();
      }

    /** Implements pesistent storage specific to instantiated representation.
        \param storage - physical storage for attachment, to be next saved in mail persistent
                         representation,
        \param failedFiles - list of file infos which attachment has been impossible since they
                         not exists or are unreadable.
    */
    virtual void Store(TAttachmentContainer* storage, TFileInfoList* failedFiles) const = 0;

    /** Allows to save given attachment item to specified storage.
        \param target - target when selected attachment will be saved to. If given path points to
                        already existing file, it will be overwitten.

        Returns status of save operation.
    */
    virtual TSaveStatus Save(QFile& target) const = 0;

    /** Retrieves file name being displayed. It can be different than real underlying file name when
        user rename the attachment.
    */
    QString GetDisplayedFileName() const
      {
      if(FileNameInfo != nullptr)
        return FileNameInfo->GetDisplayedFileName();

      return text();
      }

  /// QTableWidgetItem override.
    virtual void setData(int role, const QVariant& value)
      {
      QTableWidgetItem::setData(role, value);
      if(role == Qt::EditRole && value.toString().trimmed().isEmpty() == false)
        {        
        Owner->OnAttachmentItemChanged();
        }
      }

  protected:
    /// Constructor to build item for name column.
    AAttachmentItem(const QString& name, TFileAttachmentWidget* owner, AAttachmentItem* fileNameInfo) :
      QTableWidgetItem(name),
      Owner(owner),
      FileNameInfo(fileNameInfo)
      {
      assert(owner != nullptr);
      /// Only items representing name should be registered.
      AttachmentListPos = owner->AttachmentList.insert(owner->AttachmentList.end(), this);
      }

    /// Constructor to build item for size column.
    AAttachmentItem(AAttachmentItem* fileNameInfo, const TScaledSize& scaledSize) :
      QTableWidgetItem(TFileAttachmentWidget::toString(scaledSize)),
      Owner(nullptr),
      FileNameInfo(fileNameInfo) {}

    virtual ~AAttachmentItem() {}

  private:
    TFileAttachmentWidget*  GetOwner() const
      {
      return FileNameInfo != nullptr ? FileNameInfo->GetOwner() : Owner;
      }

  /// Class attributes:
  protected:
    typedef TFileAttachmentList::iterator TFileAttachmentListPos;

    TFileAttachmentWidget*  Owner;
    /// Non-null when this object represents a 'size' item.
    AAttachmentItem*        FileNameInfo;
    /// Filled in when item is attached to the list.
    TFileAttachmentListPos  AttachmentListPos;
  };

/** Represents attachment table widget item holding additional data needed to simplify
    operations performed over table items.
    This implemention is dedicated to items built from physical files ie attached from disk.
*/
class TFileAttachmentWidget::TFileAttachmentItem : public AAttachmentItem
  {
  public:
    /// Constructor to build item representing file name cell.
    TFileAttachmentItem(const QFileInfo& fileInfo, TFileAttachmentWidget* owner) :
      AAttachmentItem(fileInfo.fileName().toStdString().c_str(), owner, nullptr),
      FileInfo(fileInfo)
      {
      owner->TotalAttachmentSize += fileInfo.size();
      setToolTip(fileInfo.absoluteFilePath());
      }

    /// Constructor to build file size cell.
    TFileAttachmentItem(TFileAttachmentItem* fileNameItem, const TScaledSize& scaledSize) :
      AAttachmentItem(fileNameItem, scaledSize),
      FileInfo(fileNameItem->FileInfo) {}

    virtual ~TFileAttachmentItem() {}

  /// AAttachmentItem class reimplementation:
    /// \see AAttachmentItem description.
    virtual void Unregister() override
      {
      if(Owner != nullptr)
        {
        TFileInfoList::iterator foundPos = std::find(Owner->AttachmentIndex.begin(),
          Owner->AttachmentIndex.end(), FileInfo);
        Owner->AttachmentIndex.erase(foundPos);

        Owner->TotalAttachmentSize -= FileInfo.size();
        }

      AAttachmentItem::Unregister();
      }

    /// \see AAttachmentItem description.
    virtual void Store(TAttachmentContainer* storage, TFileInfoList* failedFiles) const override
      {
      assert(storage != nullptr);
      assert(failedFiles != nullptr);

      const QFileInfo& fileInfo = FileInfo;

      QFile file(fileInfo.absoluteFilePath());
      bool success = file.open(QIODevice::ReadOnly);
      if(success)
        {
        storage->push_back(TPhysicalAttachment());
        TPhysicalAttachment& physicalAttachment = storage->back();

        physicalAttachment.filename = GetDisplayedFileName().toStdString();
        qint64 fileSize = file.size();
        physicalAttachment.body.resize(fileSize);
        qint64 readLen = file.read(physicalAttachment.body.data(), fileSize);
        file.close();

        if(readLen != fileSize)
          {
          /// File read was incomplete. Mark it as failure and remove from physical attachment.
          storage->pop_back();
          failedFiles->push_back(fileInfo);
          }
        }
      else
        {
        failedFiles->push_back(fileInfo);
        }
      }
    
    /// \see AAttachmentItem description.
    virtual TSaveStatus Save(QFile& target) const
      {
      QFileInfo targetFI(target);

      if(FileInfo.isReadable() == false)
        return TSaveStatus::READ_SOURCE_ERROR;

      if(FileInfo == targetFI)
        return TSaveStatus::SUCCESS;

      QFile source(FileInfo.absoluteFilePath());

      if(source.open(QFile::ReadOnly) == false)
        return TSaveStatus::READ_SOURCE_ERROR;

        /** We assume the user would like to overwrite specified target (it should be asked earlier
            when selecting target file).
        */
      if(target.open(QFile::WriteOnly) == false)
        {
        source.close();
        return TSaveStatus::WRITE_TARGET_ERROR;
        }

      qint64 sourceSize = source.size();
      qint64 readSize = 0;

      const size_t bufferSize = 4*1024*1024;
      std::vector<char> buffer(bufferSize);

      TSaveStatus saveStatus = TSaveStatus::SUCCESS;

      do
        {
        qint64 toBeRead = sourceSize - readSize;
        qint64 readBytes = source.read(buffer.data(), toBeRead > bufferSize ? bufferSize : toBeRead);
        if(readBytes == -1)
          {
          saveStatus = TSaveStatus::READ_SOURCE_ERROR;
          break;
          }

        qint64 writtenBytes = target.write(buffer.data(), readBytes);
        if(readBytes != writtenBytes)
          {
          saveStatus = TSaveStatus::WRITE_TARGET_ERROR;
          break;
          }

        readSize += readBytes;
        }
      while(sourceSize > readSize);

      source.close();
      target.close();

      return saveStatus;
      }

  /// QTableWidgetItem override.
    virtual TFileAttachmentItem* clone() const override
      {
      TFileAttachmentItem* cloned = new TFileAttachmentItem(*this);
      
      /// Cloned item should not be unregistered from containers.
      cloned->FileNameInfo = nullptr;
      cloned->Owner = nullptr;
      cloned->AttachmentListPos = TFileAttachmentListPos();
      
      return cloned;
      }

  /// Class attributes:
  private:
    QFileInfo FileInfo;
  };

/** Represents attachment item built from already existing mail message contents, for example
    by opening Draft mail to further edit purposes or to view received mail in read only mode.
*/
class TFileAttachmentWidget::TVirtualAttachmentItem : public AAttachmentItem
  {
  public:
    /// Constructor to represent file name cell.
    TVirtualAttachmentItem(const TPhysicalAttachment& sourceData, TFileAttachmentWidget* owner) :
      AAttachmentItem(sourceData.filename.c_str(), owner, nullptr),
      Data(sourceData)
      {
      owner->TotalAttachmentSize += Data.body.size();
      }

    TVirtualAttachmentItem(TVirtualAttachmentItem* fileNameItem, const TScaledSize& scaledSize) :
      AAttachmentItem(fileNameItem, scaledSize) {}

    virtual ~TVirtualAttachmentItem() {}

  /// AAttachmentItem class reimplementation:
    /// Unregisters item from file attachment list.
    virtual void Unregister() override
      {
      if(Owner != nullptr)
        Owner->TotalAttachmentSize -= Data.body.size();

      AAttachmentItem::Unregister();
      }

    /// \see AAttachmentItem description.
    virtual void Store(TAttachmentContainer* storage, TFileInfoList* failedFiles) const override
      {
      assert(storage != nullptr);
      assert(failedFiles != nullptr);
      storage->push_back(Data);
      /// Always use name stored in the item since it could be changed.
      storage->back().filename = GetDisplayedFileName().toStdString();
      }

    /// \see AAttachmentItem description.
    virtual TSaveStatus Save(QFile& target) const override
      {
      bool success = target.open(QFile::WriteOnly);
      if(success == false)
        return TSaveStatus::WRITE_TARGET_ERROR;

      long long writtenBytes = target.write(Data.body.data(), Data.body.size());

      TSaveStatus saveStatus = TSaveStatus::SUCCESS;

      if(writtenBytes != Data.body.size())
        saveStatus = TSaveStatus::WRITE_TARGET_ERROR;

      target.close();

      return saveStatus;
      }

  /// QTableWidgetItem override.
    virtual TVirtualAttachmentItem* clone() const override
      {
      TVirtualAttachmentItem* cloned = new TVirtualAttachmentItem(*this);
      
      /// Cloned item should not be unregistered from containers.
      cloned->FileNameInfo = nullptr;
      cloned->Owner = nullptr;
      cloned->AttachmentListPos = TFileAttachmentListPos();
      
      return cloned;
      }

  /// Class attributes:
  private:
    TPhysicalAttachment Data;
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
  bool sortEnabled = FreezeAttachmentTable();

  for(const TPhysicalAttachment& a : attachedFiles)
    {
    size_t size = a.body.size();

    TScaledSize scaledSize = ScaleAttachmentSize(size);

    /// Allocate objects representing table items - name item automatically will register in the list.
    TVirtualAttachmentItem* fileNameItem = new TVirtualAttachmentItem(a, this);
    TVirtualAttachmentItem* fileSizeItem = new TVirtualAttachmentItem(fileNameItem, scaledSize);
    AddAttachmentItems(fileNameItem, fileSizeItem);
    }

  UnFreezeAttachmentTable(sortEnabled);
  
  UpdateColumnHeaders();
  }

bool TFileAttachmentWidget::GetAttachedFiles(TAttachmentContainer* storage,
  TFileInfoList* failedFilesStorage) const
  {
  assert(storage != nullptr);
  assert(failedFilesStorage != nullptr);

  storage->reserve(AttachmentList.size());

  for(const auto& item : AttachmentList)
    item->Store(storage, failedFilesStorage);

  return failedFilesStorage->empty();
  }

void TFileAttachmentWidget::ConfigureContextMenu()
  {
  QAction* sep = new QAction(this);
  sep->setSeparator(true);

  /** Register all actions to be displayed in ctx menu in a table widget (it should have configured
      ContextMenuPolicy to ActionsContextMenu).
  */
  ui->attachmentTable->addAction(ui->actionOpen);
  ui->attachmentTable->addAction(ui->actionSave);
  ui->attachmentTable->addAction(sep);

  ui->attachmentTable->addAction(ui->actionAdd);
  ui->attachmentTable->addAction(ui->actionDel);
  ui->attachmentTable->addAction(ui->actionRename);

  sep = new QAction(this);
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
  QString text = QString::number(AttachmentList.size()) + tr(" attachment(s)");
  QTableWidgetItem* header = ui->attachmentTable->horizontalHeaderItem(NAME_COLUMN_IDX);
  header->setText(text);

  TScaledSize scaledSize = ScaleAttachmentSize(TotalAttachmentSize);

  text = tr("Total attachment size: ") + toString(scaledSize);
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

void TFileAttachmentWidget::OnAttachmentItemChanged()
  {
  emit attachmentListChanged();
  }

void TFileAttachmentWidget::AddAttachmentItems(AAttachmentItem* fileNameItem,
  AAttachmentItem* fileSizeItem)
  {
  unsigned int row = AttachmentList.size();

  ui->attachmentTable->setRowCount(row);
  ui->attachmentTable->setItem(row-1, NAME_COLUMN_IDX, fileNameItem);
  ui->attachmentTable->setItem(row-1, SIZE_COLUMN_IDX, fileSizeItem);
  }

QString TFileAttachmentWidget::toString(const TScaledSize& scaledSize)
  {
  return QString::number(scaledSize.first, 'f', 1) + tr(" ") + QString(scaledSize.second);
  }

bool TFileAttachmentWidget::SaveAttachmentItem(const AAttachmentItem* item,
  const QFileInfo& targetPath, bool checkForOverwrite)
  {
  const QString title(tr("Save attachment..."));
  if(checkForOverwrite && targetPath.exists())
    {
    auto result = QMessageBox::question(this, title, tr("File: ") +
      targetPath.absoluteFilePath() + tr(" already exists.\nDo you want to overwrite it ?"));

    if(result == QMessageBox::Button::No)
      return false;
    }

  QFile targetFile(targetPath.absoluteFilePath());
  TSaveStatus saveStatus = item->Save(targetFile);

  switch(saveStatus)
    {
    case TSaveStatus::READ_SOURCE_ERROR:
      QMessageBox::warning(this, title, tr("Cannot read file: ") + item->GetDisplayedFileName());
      return false;

    case TSaveStatus::WRITE_TARGET_ERROR:
      QMessageBox::warning(this, title, tr("Cannot write to file: ") + targetPath.absoluteFilePath());
      return false;

    case TSaveStatus::SUCCESS:
      return true;

    default:
      assert(false && "Unknown save status");
    }

  return false;
  }

inline
void TFileAttachmentWidget::RetrieveSelection(TSelection* storage) const
  {
  assert(storage != nullptr);

  QList<QTableWidgetItem*> selection = ui->attachmentTable->selectedItems();

  storage->reserve(selection.size());

  /** \warning Selection contains separate item for each cell (in row) but we would like to operate
      only on these pointing to name column.
  */
  for(QTableWidgetItem* item : selection)
    {
    if(item->column() != NAME_COLUMN_IDX)
      continue;

    assert(dynamic_cast<AAttachmentItem*>(item) != nullptr);
    AAttachmentItem* fileItem = static_cast<AAttachmentItem*>(item);
    storage->push_back(fileItem);
    }
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
    AddAttachmentItems(fileNameItem, fileSizeItem);
    }

  UnFreezeAttachmentTable(sortEnabled);
  
  UpdateColumnHeaders();

  emit attachmentListChanged();
  }

void TFileAttachmentWidget::onDelTriggered()
  {
  TSelection selection;
  RetrieveSelection(&selection);

  bool sortEnabled = FreezeAttachmentTable();

  for(AAttachmentItem* item : selection)
    {
    item->Unregister();
    int rowNo = item->row();
    ui->attachmentTable->removeRow(rowNo);
    }

  UnFreezeAttachmentTable(sortEnabled);

  UpdateColumnHeaders();
  
  emit attachmentListChanged();
  }

void TFileAttachmentWidget::onOpenTriggered()
  {
  TSelection selection;
  RetrieveSelection(&selection);

  assert(selection.size() == 1 &&
    "Bad code in command update ui (onAttachementTableSelectionChanged)");

  const AAttachmentItem* item = selection.front();
  item->Open();
  }

void TFileAttachmentWidget::onSaveTriggered()
  {
  saveAttachments ();
  }

void TFileAttachmentWidget::onRenameTriggered()
  {
  TSelection selection;
  RetrieveSelection(&selection);

  assert(selection.size() == 1 &&
    "Bad code in command update ui (onAttachementTableSelectionChanged)");

  AAttachmentItem* item = selection.front();
  ui->attachmentTable->editItem(item);
  }

void TFileAttachmentWidget::onAttachementTableSelectionChanged()
  {
  TSelection selection;
  RetrieveSelection(&selection);

  unsigned int selectedCount = selection.size();
  bool anySelection = selectedCount != 0;
  bool singleSelection = selectedCount == 1;

  ui->actionAdd->setEnabled(EditMode);
  ui->actionDel->setEnabled(anySelection && EditMode);
  ui->actionOpen->setEnabled(singleSelection);
  ui->actionSave->setEnabled(anySelection);
  ui->actionRename->setEnabled(singleSelection && EditMode);
  }

void TFileAttachmentWidget::selectAllFiles()
{
  ui->attachmentTable->selectAll();
}

bool TFileAttachmentWidget::saveAttachments()
{
  TSelection selection;
  RetrieveSelection(&selection);

  QString docPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

  if(selection.size() > 1)
  {
    /// If multiple items are selected ask for directory where files should be written to.
    QString storageDir = QFileDialog::getExistingDirectory(this,
      tr("Save selected file attachments..."), docPath, QFileDialog::ShowDirsOnly);
    if(storageDir.length() == 0)
      return false;
    QDir dir(storageDir);
    bool savedOK = true;
    for(const AAttachmentItem* attachmentItem : selection)
    {
      QFileInfo fi(dir, attachmentItem->GetDisplayedFileName());
      if (SaveAttachmentItem(attachmentItem, fi, true) == false)
        savedOK = false;
    }
    return savedOK;
  }
  else
  {
    /// If single item is selected just ask for direct path to save it
    QString filePath = QFileDialog::getSaveFileName(this, "Save attachment file...", QDir(docPath).filePath(selection.front()->GetDisplayedFileName()));
    if(filePath.length() == 0)
      return false;
    const AAttachmentItem* attachmentItem = selection.front();
    QFileInfo fi(filePath);
    /// Here we don't need to check for file overwrite - it was done by QFileDialog.
    return SaveAttachmentItem(attachmentItem, fi, false);
  }
}

bool TFileAttachmentWidget::hasAttachment()
{
  return AttachmentList.size() > 0;
}
