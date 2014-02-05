#include "TableWidgetAttachments.hpp"
#include "fileattachmentwidget.hpp"

#include <QApplication>
#include <QClipboard>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>

TableWidgetAttachments::TableWidgetAttachments(QWidget *parent) :
    QTableWidget(parent),
    _readOnly (true)
{
 setAcceptDrops(true);
}

void TableWidgetAttachments::setReadOnly(bool readOnly)
{
  _readOnly = readOnly;
}

void TableWidgetAttachments::dragEnterEvent(QDragEnterEvent *event)
{
  if (_readOnly)
    return;

  QStringList stringList = getFilesPathFromMimeData( event->mimeData() );
  if (stringList.size())
    event->acceptProposedAction();
}
 
void TableWidgetAttachments::dragMoveEvent(QDragMoveEvent *event)
{
  if (_readOnly)
    return;

  event->acceptProposedAction();
}
 
void TableWidgetAttachments::dragLeaveEvent(QDragLeaveEvent *event)
{
  if (_readOnly)
    return;

  event->accept();
}

void TableWidgetAttachments::dropEvent(QDropEvent *event)
{  
  if (_readOnly)
    return;

  QStringList stringList = getFilesPathFromMimeData(event->mimeData());
  if (stringList.size())
  {
    emit dropEvent(stringList);
    event->acceptProposedAction();
  }
}

QStringList TableWidgetAttachments::getFilesPathFromClipboard()
{
  const QMimeData *md = QApplication::clipboard()->mimeData();
  QStringList stringList = getFilesPathFromMimeData(md);
  return stringList;
}

QStringList TableWidgetAttachments::getFilesPathFromMimeData(const QMimeData *md)
{
  QStringList stringList = QStringList();

  if(md)
  {
    bool en = md->hasText();
    if (md->hasUrls())
    {
      for (int i = 0; i < md->urls().size(); i++)
      {
        QUrl url = md->urls().at(i);      
        if (url.isLocalFile ())
        {
          QString fileName = url.toLocalFile ();
          QFileInfo fileInfo(fileName);
          if (fileInfo.isFile())
            stringList.push_back( fileName );
        }
      }
    }
  }

  return stringList;
}