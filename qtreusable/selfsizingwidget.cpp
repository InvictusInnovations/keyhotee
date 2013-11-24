#include "selfsizingwidget.h"
#include <QtCore>
#include <QtGui>

SelfSizingWidget::SelfSizingWidget(QWidget *parent) :
    QWidget(parent)
{
}

// this should be called in constructor of derived classes after call to setupUi
void SelfSizingWidget::readSettings()
{
    QSettings settings("Invictus Innovations","BitSharesGUI");
    settings.beginGroup(windowTitle());
    restoreGeometry(settings.value("geometry").toByteArray());
    settings.endGroup();
}

void SelfSizingWidget::writeSettings()
{
    QSettings settings("Invictus Innovations","BitSharesGUI");
    settings.beginGroup(windowTitle());
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void SelfSizingWidget::closeEvent(QCloseEvent *event)
{
    if (okToContinue())
        {
        writeSettings();
        event->accept();
        }
    else
        {
        event->ignore();
        }
}

