#ifndef TMESSAGEEDIT_HPP
#define TMESSAGEEDIT_HPP

#include <QTextEdit>

class TMessageEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit TMessageEdit(QWidget *parent = 0);
protected:
    virtual void insertFromMimeData(const QMimeData *source);

signals:
  void addAttachments(QStringList);

public slots:

};

#endif // TMESSAGEEDIT_HPP
