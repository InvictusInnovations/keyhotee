#include "TMessageEdit.hpp"
#include "qtreusable/MimeDataChecker.hpp"

TMessageEdit::TMessageEdit(QWidget *parent) :
    QTextEdit(parent)
{

}

void TMessageEdit::insertFromMimeData(const QMimeData *source)
{
  MimeDataChecker mime( source );
  QStringList files = mime.getFilesPath();
  if (files.size())
  {
    emit addAttachments(files);
  }
  else
    QTextEdit::insertFromMimeData(source);
}
