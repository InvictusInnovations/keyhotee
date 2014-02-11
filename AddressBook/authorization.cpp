#include "authorization.hpp"
#include "ui_authorization.h"

#include <KeyhoteeMainWindow.hpp>

Authorization::Authorization(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::Authorization)
{
  ui->setupUi(this);

  ui->keyhoteeidpubkey->showCopyToClipboard(false);
  ui->keyhoteeidpubkey->setEditable(false);

  connect(ui->button_accept, &QPushButton::released, this, &Authorization::onAccept);
  connect(ui->button_deny, &QPushButton::released, this, &Authorization::onDeny);
  connect(ui->button_block, &QPushButton::released, this, &Authorization::onBlock);

}

Authorization::~Authorization()
{
  delete ui;
}

void Authorization::setMsg(const bts::bitchat::decrypted_message& msg)
{
  ui->first_name->setText( "Jan" );
}

void Authorization::onAccept()
{
  getKeyhoteeWindow()->deleteCurrentAuthorizationGui();
  close();
}

void Authorization::onDeny()
{
  getKeyhoteeWindow()->deleteCurrentAuthorizationGui();
  close();
}

void Authorization::onBlock()
{
  getKeyhoteeWindow()->deleteCurrentAuthorizationGui();
  close();
}
