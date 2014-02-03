#ifndef TABLEWIDGETATTACHMENTS_H
#define TABLEWIDGETATTACHMENTS_H

#include <QTableWidget>

class TFileAttachmentWidget;

class TableWidgetAttachments : public QTableWidget
{
    Q_OBJECT
public:
    explicit TableWidgetAttachments(QWidget *parent = 0);
    virtual ~TableWidgetAttachments() {};
    QStringList getFilesPathFromClipboard();
protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dropEvent(QDropEvent *event);
private:
  QStringList getFilesPathFromMimeData(const QMimeData *md);

signals:
  void dropEvent(QStringList);
public slots:

};

#endif // TABLEWIDGETATTACHMENTS_H
