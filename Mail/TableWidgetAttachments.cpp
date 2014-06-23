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

  MimeDataChecker mime( event->mimeData() );
  if (mime.containsFiles())
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

  MimeDataChecker mime( event->mimeData() );
  QStringList stringList = mime.getFilesPath();
  if (stringList.size())
  {
    emit dropEvent(stringList);
    event->acceptProposedAction();
  }
}

void TableWidgetAttachments::focusInEvent(QFocusEvent * event)
{
  emit itemSelectionChanged();
}

QStringList TableWidgetAttachments::getFilesPathFromClipboard()
{
  MimeDataChecker mime( QApplication::clipboard()->mimeData() );
  QStringList stringList = mime.getFilesPath();
  return stringList;
}