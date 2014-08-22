#pragma once

#include "WalletsGui.hpp"

#include "fc/io/buffered_iostream.hpp"
#include "fc/filesystem.hpp"
#include "fc/thread/future.hpp"
#include "fc/interprocess/process.hpp"

class ManagedStream;
class Wallets;

class IManageWallet : public QObject
{
  Q_OBJECT

protected:
  virtual ~IManageWallet() {}

public:

  virtual void start() = 0;
  virtual bool sendCommand(const std::string& str, const std::string& expected_resp) = 0;
  virtual bool shutdown() = 0;

  virtual bool isLaunched() const = 0;
  virtual void loadPage() = 0;
  virtual Wallets* getWebWallet() const = 0;
  virtual bool isServerType(WalletsGui::ServerType type) const = 0;

signals:
  void notification(const QString& str);
};

class AManageWallet : public IManageWallet
{
public:
  virtual Wallets* getWebWallet() const override { return _wallet_web; }
  virtual bool isServerType(WalletsGui::ServerType type) const override { return _server.type == type; }

protected:
  virtual ~AManageWallet() {}

protected:
  Wallets*            _wallet_web;
  WalletsGui::Server  _server;
};

class ManageBitShares : public AManageWallet
{
public:
  ManageBitShares(Wallets* walletWeb, const WalletsGui::Server& server);
  ~ManageBitShares();
  
  virtual void start() override;
  virtual bool sendCommand(const std::string& command, const std::string& expected_resp) override;
  virtual bool shutdown() override;

  virtual bool isLaunched() const override;
  virtual void loadPage() override;

private:
  void startBitsharesClient();
  bool waitFor(const std::string& str);

  void waitAndLoadPage();

private:
  fc::path                  _app_path;
  std::vector<std::string>  _args;
  fc::future<void>          _future;
  fc::buffered_ostream_ptr  _in_stream;
  fc::buffered_istream_ptr  _out_stream;

  fc::process               _bitshares_client;
  ManagedStream*            _out_err_stream;

  bool                      _server_ready;

  QString                   _rpc_username;
  QString                   _rpc_password;
};

class ManageOtherWallet : public AManageWallet
{
public:
  ManageOtherWallet(Wallets* walletWeb, const WalletsGui::Server& server);
  ~ManageOtherWallet() {};

  virtual void start() override {};
  virtual bool sendCommand(const std::string& str, const std::string& expected_resp) override { return false; }
  virtual bool shutdown() override { return true; }

  virtual bool isLaunched() const override { return true; }
  virtual void loadPage() override;
};

