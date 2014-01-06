#include "FileAttachmentDialog.hpp"
#include "ui_FileAttachmentDialog.h"

#include "fileattachmentwidget.hpp"

FileAttachmentDialog::FileAttachmentDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileAttachmentDialog)
{
  ui->setupUi(this);
  _fileAttachment = ui->fileAttachementWidget;
}

FileAttachmentDialog::~FileAttachmentDialog()
{
    delete ui;
}

void FileAttachmentDialog::loadAttachments(const IMailProcessor::TPhysicalMailMessage& srcMsg)
{
  _fileAttachment->LoadAttachedFiles(srcMsg.attachments);
  _fileAttachment->selectAllFiles ();
}

void FileAttachmentDialog::saveAttachments()
{
  if (_fileAttachment->saveAttachments () == false)
    close();
}