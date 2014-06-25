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
    void setReadOnly(bool readOnly);
protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dropEvent(QDropEvent *event);

    virtual void focusInEvent(QFocusEvent * event) override;

signals:
  void dropEvent(QStringList);
public slots:

private:
  bool _readOnly;

};

#endif // TABLEWIDGETATTACHMENTS_H
