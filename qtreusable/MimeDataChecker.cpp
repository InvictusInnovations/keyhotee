#pragma once

#include "MimeDataChecker.hpp"

#include <QFileInfo>
#include <QMimeData>
#include <QUrl>

MimeDataChecker::MimeDataChecker(const QMimeData* mimeData)
{
	_mimeData = mimeData;
}


bool MimeDataChecker::isFiles()
{
  QStringList files =  getFilesPath();
  if (files.size())
    return true;

  return false;
}

QStringList MimeDataChecker::getFilesPath()
{
  QStringList stringList = QStringList();

  if(_mimeData)
  {
    bool en = _mimeData->hasText();
    if (_mimeData->hasUrls())
    {
      for (int i = 0; i < _mimeData->urls().size(); i++)
      {
        QUrl url = _mimeData->urls().at(i);      
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
