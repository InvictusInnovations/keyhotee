#pragma once

#include <QStringList>
class QMimeData;

class MimeDataChecker
{
public:
  MimeDataChecker(const QMimeData* mimeData);

  QStringList getFilesPath();
  bool isFiles();

private:
	const QMimeData* _mimeData;
	
};

