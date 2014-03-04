#include "TMessageEdit.hpp"
#include "maileditorwindow.hpp"

#include <bts/bitchat/bitchat_private_message.hpp>

#include "qtreusable/MimeDataChecker.hpp"

#include <QUrl>

TMessageEdit::TMessageEdit(QWidget *parent) :
    QTextEdit(parent)
{

}

void TMessageEdit::insertFromMimeData(const QMimeData *source)
{
  MimeDataChecker mime( source );
  QStringList files = mime.getFilesPath();
  if (files.size())
  {
    emit addAttachments(files);
  }
  else
    QTextEdit::insertFromMimeData(source);
}

void TMessageEdit::loadContents (const QString& body, const Attachments& attachments)
{
  QTextDocument *doc = new QTextDocument();
  //Don't eat multiple whitespaces
  doc->setDefaultStyleSheet("p, li { white-space: pre-wrap; }");
  doc->setHtml( body );
  this->setDocument (doc);

  QImage  image;
  uchar  *imageData;
  int     imageSize;
  QString imageName;
  
  int i = 0;
  for (const auto& attachment : attachments)
  {
    // image ID for resource
    imageName = QString("imageName.%1").arg(i);
	  imageData = (uchar*)attachment.body.data ();
	  imageSize = attachment.body.size();

	  //QImageReader::supportedImageFormats()    
	  bool loadOk = image.loadFromData(imageData, imageSize);
    if (loadOk)
    {
      doc->addResource( QTextDocument::ImageResource, QUrl( imageName ), image);
      QString attachmentFileName = "<br/><hr><font color=""grey"">" + 
                                    QString(attachment.filename.c_str()) + 
                                    "</font><br/>";
      this->append(attachmentFileName +  "<center><img src='" + imageName + "'></center>");
    }
    ++i;
  }

  //no set modified flag when is loading contents
  this->document()->setModified(false);
}