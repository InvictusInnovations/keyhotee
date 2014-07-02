#pragma once

#include <QList>
#include <QObject>
#include <QString>
#include <QVector>

class QWidget;

/**
  * WalletsGui stores GUI data displayed in the wallets tree 
  * in the KeyhoteeMainWindow
  */
class WalletsGui : public QObject
{
  Q_OBJECT
  /// Register enums for name use at runtime
  Q_ENUMS(ServerType)
public:
  enum ServerType { BitsharesClient, other };
  struct Server
  {
    ServerType        type;    
    uint              port;
    QString           path;
    QVector<QString>  arg; ///command line arguments
  };
  struct Data
  {
    QString name;
    QString iconPath;
    QString url;
    Server  server;
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

