#include "TImage.hpp"
#include <QBuffer>


TImage::TImage() : QImage()
{
}

/** Supporting images in ToolTips
    Usage: 
    TImage img(...);
    widget->setToolTip (img.toHtml());
*/
QString TImage::toHtml() 
{
  if (this->isNull ())
    return QString();
  
  QImage tmpImage = this->scaledToHeight (150, Qt::SmoothTransformation);
  QByteArray ba;
  QBuffer buffer(&ba);
  buffer.open(QIODevice::WriteOnly);
  tmpImage.save(&buffer, "PNG");
  return QString("<html><img src=\"data:image/png;base64,%1\"></html>").arg(QString(buffer.data().toBase64()));
}
