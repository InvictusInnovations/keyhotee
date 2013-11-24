#ifndef SELFSIZINGWIDGET_H
#define SELFSIZINGWIDGET_H

#include <QWidget>

class SelfSizingWidget : public QWidget
{
    Q_OBJECT

protected:
    virtual void readSettings();
    virtual void writeSettings();
    virtual void closeEvent(QCloseEvent *event);
    virtual bool okToContinue() { return true; }

public:
    explicit SelfSizingWidget(QWidget *parent = 0);
    
signals:
    
public slots:
    
};

#endif // SELFSIZINGWIDGET_H
