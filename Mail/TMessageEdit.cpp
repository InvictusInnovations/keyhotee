#include "TMessageEdit.hpp"

#include "qtreusable/MimeDataChecker.hpp"

TMessageEdit::TMessageEdit(QWidget *parent) : QTextBrowser(parent) {}

void TMessageEdit::insertFromMimeData(const QMimeData *source)
{
  MimeDataChecker mime( source );
  QStringList files = mime.getFilesPath();
  if (files.isEmpty())
    QTextEdit::insertFromMimeData(source);
  else
    emit attachmentAdded(files);
}

QVariant TMessageEdit::loadResource(int type, const QUrl& url)
{
if(type == QTextDocument::ImageResource && url.isLocalFile() == false)
  return QVariant(QPixmap(":/qt-project.org/styles/commonstyle/images/filecontents-32.png"));

return QTextBrowser::loadResource(type, url);
}

