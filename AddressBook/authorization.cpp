#include "authorization.hpp"
#include "ui_authorization.h"

#include <KeyhoteeMainWindow.hpp>

#include <QToolBar>

Authorization::Authorization(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::Authorization)
{
  ui->setupUi(this);

  _owner_item = nullptr;

  ui->keyhoteeidpubkey->showCopyToClipboard(false);
  ui->keyhoteeidpubkey->setEditable(false);

  _toolbar = new QToolBar(ui->toolbar_container);
  QGridLayout* grid_layout = new QGridLayout(ui->toolbar_container);
  grid_layout->setContentsMargins(0, 0, 0, 0);
  grid_layout->setSpacing(0);
  ui->toolbar_container->setLayout(grid_layout);
  grid_layout->addWidget(_toolbar, 0, 0);
  
  _accept = new QAction( QIcon( ":/images/request_accept.png"), tr("Accept"), this);
  _deny = new QAction( QIcon( ":/images/request_deny.png"), tr("Deny"), this);  
  _block = new QAction( QIcon(":/images/request_block.png"), tr("_block"), this);
  
  _toolbar->addAction(_accept);  
  _toolbar->addAction(_deny);
  _toolbar->addAction(_block);

  QLabel *label = new QLabel((tr("     Response for authorization request")));
  _toolbar->addWidget (label);
  QFont font;  
  font.setBold(true);
  font.setPointSize (16);
  label->setFont (font);

  connect(_accept, &QAction::triggered, this, &Authorization::onAccept);
  connect(_deny, &QAction::triggered, this, &Authorization::onDeny);
  connect(_block, &QAction::triggered, this, &Authorization::onBlock);

  // setting the background color of the frame so that the window looked like a window "create new contact"
  QPalette palette = ui->frame->palette();
  palette.setColor(backgroundRole(), QGuiApplication::palette().base().color());
  ui->frame->setPalette(palette);
}

Authorization::~Authorization()
{
  delete ui;
}

void Authorization::setMsg(const bts::bitchat::decrypted_message& msg)
{
  ui->first_name->setText( "Jan" );
}

void Authorization::setOwnerItem(AuthorizationItem* item)
{
  _owner_item = item;
}

void Authorization::onAccept()
{
  getKeyhoteeWindow()->deleteAuthorizationItem(_owner_item);
  close();
}

void Authorization::onDeny()
{
  getKeyhoteeWindow()->deleteAuthorizationItem(_owner_item);
  close();
}

void Authorization::onBlock()
{
  getKeyhoteeWindow()->deleteAuthorizationItem(_owner_item);
  close();
}
