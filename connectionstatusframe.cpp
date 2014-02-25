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
  const unsigned int tooltipShowTime = 1500; /// ms

  unsigned int count = DataSource.GetPeerConnectionCount();
  bool isMailConnected = DataSource.IsMailConnected();

  if(isMailConnected)
    ui->mailConnectionLbl->setPixmap(QPixmap(":/images/24x24/mailconnectionstatus.png"));
  else
    ui->mailConnectionLbl->setPixmap(QPixmap(":/images/24x24/no_mailconnectionstatus.png"));

  if(PrevStatus.second != isMailConnected)
    {
    QString txt = isMailConnected ? tr("Connection to mailserver succeeded...") :
      tr("Connection to mailserver has been lost...");

    QPoint pos = ui->mailConnectionLbl->pos();
    QToolTip::showText(mapToGlobal(pos), txt, this, QRect(), tooltipShowTime);
    }

  QString      txt;
  txt.setNum(count);
  ui->countLbl->setText(txt);

  if (count > 0)
    ui->iconLbl->setPixmap(QPixmap(":/images/16x16/connectionstatus.png"));
  else
    ui->iconLbl->setPixmap(QPixmap(":/images/16x16/no_connectionstatus.png"));

  if(PrevStatus.first != count)
    {
    QString txt = PrevStatus.first == 0 ? tr("Connection to Bitshares network succeeded...") :
      tr("Connection to Bitshares network has been lost...");

    QPoint pos = ui->iconLbl->pos();
    QToolTip::showText(mapToGlobal(pos), txt, this, QRect(), tooltipShowTime);
    }

  PrevStatus.first = count;
  PrevStatus.second = isMailConnected;
  }

