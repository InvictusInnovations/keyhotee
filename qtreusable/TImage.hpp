#ifndef TIMAGE_H
#define TIMAGE_H

#include <QImage>

class TImage : public QImage
{
public:
    TImage();
    QString toHtml();
};

#endif // TIMAGE_H
