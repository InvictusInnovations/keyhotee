#include "BlockedContentAlert.hpp"
#include "ui_BlockedContentAlert.h"

#include "BlockerDelegate.hpp"

BlockedContentAlert::BlockedContentAlert(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BlockedContentAlert),
    _blocker(nullptr)
{
  ui->setupUi(this);
  /// Set background color
  QPalette p(palette());
  p.setColor(QPalette::Window, Qt::white);
  setPalette(p);
  /// Set text color
  p.setColor(QPalette::WindowText, Qt::red);
  setPalette(p);

  hide();
  connect(ui->showRemoteContent, SIGNAL(clicked()), this, SLOT(onShowRemoteContent()));
}

BlockedContentAlert::~BlockedContentAlert()
{
    delete ui;
}

void BlockedContentAlert::initial(IBlockerDelegate* blocker)
{
  _blocker = blocker;
}

void BlockedContentAlert::onShowRemoteContent()
{
  Q_ASSERT(_blocker != nullptr);

  _blocker->onLoadBlockedImages();
}
