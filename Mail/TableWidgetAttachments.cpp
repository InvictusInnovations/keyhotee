#include "TableWidgetAttachments.hpp"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>

TableWidgetAttachments::TableWidgetAttachments(QWidget *parent) :
    QTableWidget(parent)
{
}

bool TableWidgetAttachments::dropMimeData(int row, int column, const QMimeData *data, Qt::DropAction action)
{
  return QTableWidget::dropMimeData(row, column, data, action);
}

Qt::DropActions TableWidgetAttachments::supportedDropActions() const
{
  Qt::DropActions dropActions = QTableWidget::supportedDropActions();

  return dropActions;
}

QStringList TableWidgetAttachments::getFilesPathFromClipboard()
{
  const QMimeData *md = QApplication::clipboard()->mimeData();
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