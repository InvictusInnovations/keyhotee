#pragma once

#include <QList>
#include <QString>

class QWidget;

/**
  * WalletsGui stores GUI data displayed in the wallets tree 
  * in the KeyhoteeMainWindow
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

  WalletsGui(QWidget* parent);
  virtual ~WalletsGui(){};

  /// @returns wallets data displayed in the KeyhoteeMainWindow
  const QList<Data>& getData() const;

private:
  /// Read wallets GUI data
  void read(QWidget* parent);

private:
  QList<Data> _data;

};

