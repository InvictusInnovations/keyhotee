#ifndef SELFSIZINGMAINWINDOW_H
#define SELFSIZINGMAINWINDOW_H

#include <QMainWindow>

class SelfSizingMainWindow : public QMainWindow
{
    Q_OBJECT
    QString _settings_file;

protected:
            void setSettingsFile(QString settings_file) { _settings_file = settings_file; }
    virtual void readSettings();
    virtual void writeSettings();

public:
    explicit SelfSizingMainWindow(QWidget *parent = 0);
};

#endif // SELFSIZINGMAINWINDOW_H
