#ifndef TIMAGE_H
#define TIMAGE_H

#include <QImage>

class TImage : public QImage
{
public:
    explicit TImage(uchar *data, int size);
    QString toHtml();
};

#endif // TIMAGE_H
