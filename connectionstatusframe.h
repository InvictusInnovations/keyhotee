#ifndef CONNECTIONSTATUSFRAME_H
#define CONNECTIONSTATUSFRAME_H

#include <QFrame>

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
    const IConnectionStatusDataSource& DataSource;
    Ui::ConnectionStatusFrame *ui;
  };

#endif // CONNECTIONSTATUSFRAME_H
