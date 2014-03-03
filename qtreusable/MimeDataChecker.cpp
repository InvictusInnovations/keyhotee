#include "MimeDataChecker.hpp"

#include <QFileInfo>
#include <QMimeData>
#include <QUrl>

MimeDataChecker::MimeDataChecker(const QMimeData* mimeData)
{
  _mimeData = mimeData;
}

bool MimeDataChecker::containsFiles() const
{
  if(_mimeData == nullptr || _mimeData->hasUrls() == false)
    return false;

  QList<QUrl> urlList = _mimeData->urls();

  for (const QUrl& url : urlList)
  {
    if (url.isLocalFile ())
    {
      QString fileName = url.toLocalFile ();
      QFileInfo fileInfo(fileName);
      if (fileInfo.isFile())
        return true;
    }
  }

  return false;
}

QStringList MimeDataChecker::getFilesPath() const
{
  QStringList stringList;

  if(_mimeData == nullptr || _mimeData->hasUrls() == false)
    return stringList;

  QList<QUrl> urlList = _mimeData->urls();

  for (const QUrl& url : urlList)
  {
    if (url.isLocalFile ())
    {
      QString fileName = url.toLocalFile ();
      QFileInfo fileInfo(fileName);
      if (fileInfo.isFile())
        stringList.push_back( fileName );
    }
  }

  return stringList;
}
