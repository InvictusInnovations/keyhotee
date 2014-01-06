#ifndef FILEATTACHMENTDIALOG_HPP
#define FILEATTACHMENTDIALOG_HPP

#include "ch/mailprocessor.hpp"

#include <QDialog>

class TFileAttachmentWidget;

namespace Ui {
class FileAttachmentDialog;
}

class FileAttachmentDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FileAttachmentDialog(QWidget *parent = 0);
  ~FileAttachmentDialog();

public:
  void loadAttachments(const IMailProcessor::TPhysicalMailMessage& srcMsg);
  void saveAttachments();

private:
    Ui::FileAttachmentDialog  *ui;
    TFileAttachmentWidget     *_fileAttachment;
};

#endif // FILEATTACHMENTDIALOG_HPP
