#ifndef TABLEWIDGETATTACHMENTS_H
#define TABLEWIDGETATTACHMENTS_H

#include <QTableWidget>

class TableWidgetAttachments : public QTableWidget
{
    Q_OBJECT
public:
    explicit TableWidgetAttachments(QWidget *parent = 0);
    QStringList getFilesPathFromClipboard();
protected:
    virtual bool dropMimeData(int row, int column, const QMimeData *data, Qt::DropAction action);
    virtual Qt::DropActions supportedDropActions() const;

signals:

public slots:

};

#endif // TABLEWIDGETATTACHMENTS_H
