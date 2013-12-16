#include "wallets.hpp"
#include "ui_wallets.h"

Wallets::Wallets(QWidget* parent) :
  QWidget(parent),
  ui(new Ui::Wallets)
  {
  ui->setupUi(this);
  //setupActions();
  }

Wallets::~Wallets()
  {
  delete ui;
  }

