#pragma once

#include <QStringList>
class QMimeData;

class MimeDataChecker
{
public:
  explicit MimeDataChecker(const QMimeData* mimeData);

  QStringList getFilesPath() const;
  bool containsFiles() const;

private:
  const QMimeData* _mimeData;
};

