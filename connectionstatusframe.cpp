#include "connectionstatusframe.h"
#include "ui_connectionstatusframe.h"

#include "ch/connectionstatusds.h"

#include <QTimer>

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
  unsigned int count = DataSource.GetConnectionCount();

  QString txt;
  txt.setNum(count);
  ui->countLbl->setText(txt);

  if(count > 0)
    ui->iconLbl->setPixmap(QPixmap(":/images/16x16/connectionstatus.png"));
  else
    ui->iconLbl->setPixmap(QPixmap(":/images/16x16/no_connectionstatus.png"));
  }

