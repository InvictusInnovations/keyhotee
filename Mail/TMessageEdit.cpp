#include "TMessageEdit.hpp"

#include "qtreusable/MimeDataChecker.hpp"

#include <QUrl>

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
//if(type == QTextDocument::ImageResource && url.isLocalFile() == false)
//  return QVariant(QPixmap(":/qt-project.org/styles/commonstyle/images/filecontents-32.png"));

return QTextBrowser::loadResource(type, url);
}

void TMessageEdit::loadContents (const QString& body, const TAttachmentContainer& attachments)
{
  QTextDocument *doc = document();
  doc->clear();
  doc->setHtml( body );

  unsigned int i = 0;
  for (const auto& attachment : attachments)
  {
    // image ID for resource
    QString imageName(QString("imageName.%1").arg(i));

    const uchar* imageData = (const uchar*)attachment.body.data();
    size_t imageSize = attachment.body.size();

    //QImageReader::supportedImageFormats()
    QImage image;
    bool loadOk = image.loadFromData(imageData, imageSize);
    if (loadOk)
    {
      doc->addResource( QTextDocument::ImageResource, QUrl( imageName ), image);
      QString attachmentFileName = "<br/><hr/><font color=""grey"">" + 
                                    QString(attachment.filename.c_str()) + 
                                    "</font><br/>";
      this->append(attachmentFileName +  "<center><img src='" + imageName + "'></center>");
    }
    ++i;
  }

  //no set modified flag when is loading contents
  doc->setModified(false);
}

