#pragma once

#include <QIcon>
#include <QList>
#include <QString>

/**
  * WalletsGui stores GUI data
  */
class WalletsGui 
{
public:
  struct Data
  {
    QString name;
    QString iconPath;
    QString url;
  };

  WalletsGui(){};
  WalletsGui(QWidget* parent);
  //virtual ~WalletsGui();

private:
  /// Read wallets GUI data
  void read(QWidget* parent);

private:
  QList	<Data> data;

};

