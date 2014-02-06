#include "TableWidgetAttachments.hpp"
#include "fileattachmentwidget.hpp"
#include "qtreusable/MimeDataChecker.hpp"

#include <QApplication>
#include <QClipboard>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>

TableWidgetAttachments::TableWidgetAttachments(QWidget *parent) :
    QTableWidget(parent)
{
 setAcceptDrops(true);
}

void TableWidgetAttachments::dragEnterEvent(QDragEnterEvent *event)
{
  MimeDataChecker mime( event->mimeData() );
  if (mime.isFiles())
    event->acceptProposedAction();
}
 
void TableWidgetAttachments::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}
 
void TableWidgetAttachments::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void TableWidgetAttachments::dropEvent(QDropEvent *event)
{  
  MimeDataChecker mime( event->mimeData() );
  QStringList stringList = mime.getFilesPath();
  if (stringList.size())
  {
    emit dropEvent(stringList);
    event->acceptProposedAction();
  }
}

QStringList TableWidgetAttachments::getFilesPathFromClipboard()
{
  MimeDataChecker mime( QApplication::clipboard()->mimeData() );
  QStringList stringList = mime.getFilesPath();
  return stringList;
}