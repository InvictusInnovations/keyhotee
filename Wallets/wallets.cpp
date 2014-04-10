#include "wallets.hpp"
#include "ui_wallets.h"
#include "transactionhistorymodel.hpp"

Wallets::Wallets(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::Wallets)
{
    ui->setupUi(this);
}

Wallets::~Wallets()
{
    delete ui;
}

