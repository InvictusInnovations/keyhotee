#include "connectionstatusframe.h"
#include "ui_connectionstatusframe.h"

#include "ch/connectionstatusds.h"

#include <QTimer>
#include <QToolTip>

TConnectionStatusFrame::TConnectionStatusFrame(const IConnectionStatusDataSource& ds) :
  QFrame(nullptr),
  DataSource(ds),
  ui(new Ui::ConnectionStatusFrame)
  {
  ui->setupUi(this);

  MailConnSourceTooltip = ui->mailConnectionLbl->toolTip();
  PeerConnSourceTooltip = ui->iconLbl->toolTip();

  updateConnectionStatus(true);

  QTimer* statusUpdateTimer = new QTimer(this);
  connect(statusUpdateTimer, SIGNAL(timeout()), this, SLOT(updateConnectionStatus()));
  statusUpdateTimer->start(2000); // update it every 2 seconds
  }

TConnectionStatusFrame::~TConnectionStatusFrame()
  {
  delete ui;
  }

void TConnectionStatusFrame::updateConnectionStatus()
  {
  updateConnectionStatus(false);
  }

void TConnectionStatusFrame::updateConnectionStatus(bool initialUpdate)
  {
  const unsigned int tooltipShowTime = 1500; /// ms

  unsigned int count = DataSource.GetPeerConnectionCount();
  bool isMailConnected = DataSource.IsMailConnected();

  if(isMailConnected)
    ui->mailConnectionLbl->setPixmap(QPixmap(":/images/24x24/mailconnectionstatus.png"));
  else
    ui->mailConnectionLbl->setPixmap(QPixmap(":/images/24x24/no_mailconnectionstatus.png"));

  QString txt;

  if(PrevStatus.second != isMailConnected || initialUpdate)
    {
    QString tooltip(MailConnSourceTooltip);

    if(isMailConnected)
      {
      tooltip += " (<b>";
      tooltip += tr("Connected");
      tooltip += "</b>)";
      txt = tr("Connection to mailserver succeeded...");
      }
    else
      {
      tooltip += " (<b>";
      tooltip += tr("Disconnected");
      tooltip += "</b>)";

      txt = tr("Connection to mailserver has been lost...");
      }

    ui->mailConnectionLbl->setToolTip(tooltip);

    QPoint pos = ui->mailConnectionLbl->pos();
    /// Don't pass here parent widget to avoid switching active window when tooltip appears
    QToolTip::showText(mapToGlobal(pos), txt, nullptr, QRect(), tooltipShowTime);
    }

  txt.setNum(count);
  ui->countLbl->setText(txt);

  if (count > 0)
    ui->iconLbl->setPixmap(QPixmap(":/images/16x16/connectionstatus.png"));
  else
    ui->iconLbl->setPixmap(QPixmap(":/images/16x16/no_connectionstatus.png"));

  if(PrevStatus.first != count || initialUpdate)
    {
    QString tooltip(PeerConnSourceTooltip);

    if(PrevStatus.first == 0 && count != 0)
      {
      txt =  tr("Connection to Bitshares network succeeded...");

      tooltip += " (<b>";
      tooltip += tr("Connected");
      tooltip += "</b>)";
      }
    else
    if(count == 0)
      {
      txt = tr("Connection to Bitshares network has been lost...");
      tooltip += " (<b>";
      tooltip += tr("Disconnected");
      tooltip += "</b>)";
      }

    ui->iconLbl->setToolTip(tooltip);

    QPoint pos = ui->iconLbl->pos();
    /// Don't pass here parent widget to avoid switching active window when tooltip appears
    QToolTip::showText(mapToGlobal(pos), txt, nullptr, QRect(), tooltipShowTime);
    }

  PrevStatus.first = count;
  PrevStatus.second = isMailConnected;
  }

