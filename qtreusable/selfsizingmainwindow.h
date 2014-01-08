#ifndef SELFSIZINGMAINWINDOW_H
#define SELFSIZINGMAINWINDOW_H

#include <QMainWindow>

struct MailSettings
{
  int sortColumnInbox;
  int sortOrderInbox;
};

class SelfSizingMainWindow : public QMainWindow
{
    Q_OBJECT
    QString _settings_file;

public:
  const MailSettings& getMailSettings() const;
protected:
            void setSettingsFile(QString settings_file) { _settings_file = settings_file; }
    virtual void readSettings();
    virtual void writeSettings();
    //virtual void closeEvent(QCloseEvent *event);
    //virtual bool okToContinue() { return true; }

public:
    explicit SelfSizingMainWindow(QWidget *parent = 0);
    
signals:
    
public slots: 
private:
  MailSettings _mailSettings;
};

#endif // SELFSIZINGMAINWINDOW_H
