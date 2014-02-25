#ifndef CONNECTIONSTATUSFRAME_H
#define CONNECTIONSTATUSFRAME_H

#include <QFrame>

#include <utility>

namespace Ui {
class ConnectionStatusFrame;
}

class IConnectionStatusDataSource;

/** Encapsulates connection status controls to be placed on main window status bar.
*/
class TConnectionStatusFrame : public QFrame
  {
  Q_OBJECT

  public:
    TConnectionStatusFrame(const IConnectionStatusDataSource& dataSource);
    virtual ~TConnectionStatusFrame();

  public slots:
    /// Slot called by update timer.
    void updateConnectionStatus();

  private:
    void updateConnectionStatus(bool initialUpdate);

    typedef std::pair<unsigned int, bool> TStatus;
    const IConnectionStatusDataSource& DataSource;
    TStatus PrevStatus;
    QString MailConnSourceTooltip;
    QString PeerConnSourceTooltip;
    Ui::ConnectionStatusFrame *ui;
  };

#endif // CONNECTIONSTATUSFRAME_H
