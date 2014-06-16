#include "TMessageEdit.hpp"
#include "BlockerDelegate.hpp"

#include "qtreusable/MimeDataChecker.hpp"

#include <fc/log/logger.hpp>

#include <QDir>
#include <QEventLoop>
#include <QImageReader>
#include <QMimeData>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QTextBlockFormat>
#include <QUrl>

TMessageEdit::TMessageEdit(QWidget *parent) :
  QTextBrowser(parent),
 _imageLoadAllowed(false),
 _anyBlockedImage(false)
{

}

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
  if (_imageLoadAllowed == false && type == QTextDocument::ImageResource && url.isLocalFile() == false)
  {
    /// note: call initial() method
    Q_ASSERT(_blocker != nullptr);

    /// notify once when _anyBlockedImage is changed
    if (_anyBlockedImage == false)
      _blocker->onBlockedImage();

    _anyBlockedImage = true;    

    return QVariant(QPixmap(":/qt-project.org/styles/commonstyle/images/filecontents-32.png"));
  }
  else if (type == QTextDocument::ImageResource && url.isLocalFile() == false &&
           url.toString().startsWith("http://"))
  {
    QString remoteUrl = url.toString();

    QByteArray extension;
    extension.append(getFileExtension(remoteUrl));

    /// Check if image format is supported
    if (QImageReader::supportedImageFormats().contains(extension) == false)
    {
      return false;
    }

    /// Get local path for remote image
    QString localPath = getCachedImagePath(remoteUrl);

    bool fileExistLocally = false;
    if (QFile::exists(localPath) == false)
    {
      /// Download remote image
      if (downloadImage(remoteUrl) == false)
      {
        ilog("Failed to fetch remote image ${path}", ("path", remoteUrl.toStdString()));
        return false;
      }
      else
      {
        /// Check again is file exist locally after download
        if (QFile::exists(localPath) == true)
        {
          fileExistLocally = true;
        }
      }
    }
    else 
    {
      fileExistLocally = true;
    }

    if (fileExistLocally == true)
    {
      ilog("Local file exists ${path}", ("path", localPath.toStdString()));
      return QVariant(QPixmap(localPath));
    }

    return false;
  }

  return QTextBrowser::loadResource(type, url);
}

void TMessageEdit::loadContents(const QString& body, const TAttachmentContainer& attachments)
{
  _imageLoadAllowed = false;
  _anyBlockedImage = false;

  QTextDocument *doc = document();
  doc->clear();

  doc->setHtml( body );

  unsigned int i = 0;
  bool imageLoadAllowed = _imageLoadAllowed;
  _imageLoadAllowed = true;

  for (const auto& attachment : attachments)
  {
    // image ID for resource
    QString imageName(QString("imageName.%1").arg(i));

    const uchar* imageData = (const uchar*)attachment.body.data();
    size_t imageSize = attachment.body.size();

    QImage image;
    bool loadOk = image.loadFromData(imageData, imageSize);
    if (loadOk)
    {
      QString attachmentFileName = "<br/><hr/><font color=""grey"">" + 
                                    QString(attachment.filename.c_str()) + 
                                    "</font><br/>";
      /// Insert horizontal line
      textCursor().insertHtml(attachmentFileName);
      
      doc->addResource(QTextDocument::ImageResource, QUrl(imageName), image);

      // Insert Block
      QTextBlockFormat centerFormat;
      centerFormat.setAlignment(Qt::AlignHCenter);
      textCursor().insertBlock(centerFormat);
      // Insert image
      textCursor().insertImage(imageName);
    }
    ++i;
  }

  _imageLoadAllowed = imageLoadAllowed;

  //no set modified flag when is loading contents
  doc->setModified(false);
}

void TMessageEdit::loadBlockedImages()
  {
    if(_anyBlockedImage)
    {
      bool saveModified = document()->isModified();

      _anyBlockedImage = false;

      _imageLoadAllowed = true;
      QTextDocument* sourceDoc = document();
      QTextDocument* copy = sourceDoc->clone(this);
      setDocument(copy);
      /// FIXME - crashes after calling 'delete sourceDoc'
      ///delete sourceDoc;

      document()->setModified(saveModified);
    }
  }

QString TMessageEdit::getCachedImagePath(const QString &remoteUrl)
{
  static const QRegExp regexp("[^a-z0-9]+");
  QString url = remoteUrl.toLower();
  
  QString extension = getFileExtension(remoteUrl);  

  url.replace(regexp, "_");
  
  /// Get cache location
  QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
  cachePath += "/images";  

  /// Check if an images directory exist if not create it
  QDir dir(cachePath);
  if (dir.exists() == false)
  {
    if (dir.mkpath(cachePath) == false)
    {
      ilog("Could not create image cache path ${path}", ("path", cachePath.toStdString()));
      assert(false);
    }
  }

  return cachePath + "/" + url + "." + extension;
}


bool TMessageEdit::downloadImage(const QString &remoteUrl)
{
  //for test
  //QString remoteUrl = "http://wewewfwfdata/products/26809662/f-airpressaaaaaa-hl-425-100-400v.jpg";

  QString localPath = getCachedImagePath(remoteUrl);

  QNetworkAccessManager manager(this);
  QNetworkRequest request(remoteUrl);
  QNetworkReply *reply = manager.get(request);

  ilog("Start download of ${path}", ("path", remoteUrl.toStdString()));

  /// Wait for downloading an image
  QEventLoop loop;
  loop.connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
  loop.connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
  loop.exec();

  ilog("End download of ${path}", ("path", remoteUrl.toStdString()));

  /// Check downloading errors
  if (reply->error() != 0)
  {
    ilog("Download error: ${err}", ("err", reply->errorString().toStdString()));
    delete reply;
    return false;
  }

  /// Save remote image to local cache directory
  QFile localFile(localPath);
  if (localFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
  {
    localFile.write(reply->readAll());
    localFile.close();
    delete reply;
  }
  else
  {
    /// Can't open file
    delete reply;
    return false;
  }

  return true;  
}


QString TMessageEdit::getFileExtension(const QString &fileName)
{
  QString extension = "";
  int dot_pos = fileName.lastIndexOf('.');

  if (dot_pos != -1) 
  {
    /// +1 without dot
    extension = fileName.mid(dot_pos + 1);
  }

  return extension;
}

void TMessageEdit::initial(IBlockerDelegate* blocker)
{
  _blocker = blocker;
}