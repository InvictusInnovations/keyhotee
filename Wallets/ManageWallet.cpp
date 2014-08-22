#include "ManageWallet.hpp"

#include "ManagedStream.hpp"
#include "wallets.hpp"

#include <bts/application.hpp>

#include "fc/thread/thread.hpp"

#include <QCoreApplication>
#include <QTemporaryFile>
#include <QUuid>

extern QTemporaryFile gLogFile;

ManageBitShares::ManageBitShares(Wallets* walletWeb, const WalletsGui::Server& server)
{
  _wallet_web = walletWeb;
  _server = server;

  fc::path logpath = gLogFile.fileName().toStdWString();
  _out_err_stream = new ManagedStream(_bitshares_client, logpath);

  auto str_app_dir = QCoreApplication::applicationDirPath().toStdWString();
  fc::path app_dir(str_app_dir);
  _app_path = app_dir / server.path.toStdWString();

  auto app = bts::application::instance();
  fc::path profile_dir = app->get_configuration().data_dir.parent_path();

  foreach(QString arg, server.arg)
  {
    if(arg.contains("$keyhotee_profile"))
    {
      QString arg_new = profile_dir.string().c_str();
      arg_new.append(arg.remove("$keyhotee_profile"));
      _args.push_back(arg_new.toStdString());
    }
    else
      _args.push_back(arg.toStdString());
  }

  // On Windows it doesn't pass http basic auth credentials to rpc server
  // The problem the same as in the project qt_wallet: issue #5
  // https://github.com/BitShares/qt_wallet/issues/5
  if(1)
  {
    _rpc_username = "";
    _rpc_password = "";
  }
  else
  {
    _rpc_username = QUuid::createUuid().toString().mid(1, 36);
    _rpc_password = QUuid::createUuid().toString().mid(1, 36);
  }

  _wallet_web->setAuthentication(_rpc_username, _rpc_password);

  _server_ready = false;
}

ManageBitShares::~ManageBitShares()
{
  delete _out_err_stream;
}

void ManageBitShares::start()
{
  _future = fc::async([=](){startBitsharesClient(); });
}

bool ManageBitShares::isLaunched() const
{
  if(_future.valid() && !_future.ready())
    return true;
  else
    return false;
}

void ManageBitShares::loadPage()
{
  getWebWallet()->loadPage();
  if(!isLaunched())
    start();
  fc::async([=](){this->waitAndLoadPage(); });
}

bool ManageBitShares::sendCommand(const std::string& command, const std::string& expected_resp)
{
  ilog("write to bitshares_client: ${str}", ("str", command));

  if(!isLaunched())
  {
    wlog("bitshares_client not launched");
    return false;
  }

  if(!expected_resp.empty())
    _out_err_stream->stop_log_stdout();

  int ret = _in_stream->writesome(command.data(), command.size());
  _in_stream->writesome("\r\n", 2);
  _in_stream->flush();

  if(!expected_resp.empty())
  {
    int ret = waitFor(expected_resp);
    _out_err_stream->log_stdout_to_file();
    return ret;
  }

  return false;
}

bool ManageBitShares::shutdown()
{
  sendCommand("stop", "");
  int time = 0;
  while(_future.valid() && !_future.ready())
  {
    fc::usleep(fc::milliseconds(100));
    time++;
    // timeout 120 seconds
    if(time == 120 * 10)
      break;
  }

  if(time == 120 * 10)
  {
    if(_future.valid() && !_future.ready())
    {
      _bitshares_client.kill();
      wlog("BitShares_client process killed");
    }
  }

  emit notification(tr("Bitshares Client shutdown."));
  return true;
}

void ManageBitShares::startBitsharesClient()
{
  try
  {
    ilog("start bitshares_client: ${_app_path}", ("_app_path", _app_path));
    emit notification(tr("Starting Bitshares Client..."));
    _bitshares_client.exec(_app_path, _args, _app_path.parent_path(),
      fc::iprocess::open_all | fc::iprocess::suppress_console);

    _out_err_stream->log_stderr_to_file();
    _in_stream = _bitshares_client.in_stream();
    _out_stream = _bitshares_client.out_stream();

    waitFor("(wallet closed) >>>");

    ilog("config bitshares_client");
    std::string arg1 = "rpc_set_username " + _rpc_username.toStdString();
    sendCommand(arg1, "rpc_set_username(");

    std::string arg2 = "rpc_set_password " + _rpc_password.toStdString();
    sendCommand(arg2, "rpc_set_password(");

    ilog("starting http server");
    std::string command = "http_start_server ";
    command += std::to_string(_server.port);
    sendCommand(command, "http_server_port: ");
    ilog("http server OK");

    _server_ready = true;

    emit notification(tr("Bitshares Client launched."));
  }
  catch(...)
  {
    _server_ready = false;
    wlog("Bitshares Client NOT launched");
    emit notification(tr("Bitshares Client NOT launched."));
    return;
  }

  _bitshares_client.result();
  ilog("Bitshares Client shutdown");
}

bool ManageBitShares::waitFor(const std::string& str)
{
  std::size_t found;
  char buf[1024];
  uint readed;
  try
  {
    for(;;)
    {
      readed = _out_stream->readsome(buf, sizeof(buf));
      _out_err_stream->log_to_file(buf, readed);
      found = std::string(buf).find(str);
      if(found != std::string::npos)
        return true;
    }
  }
  catch(fc::eof_exception&)
  {
    _server_ready = false;
    wlog("eof - ${str}", ("str", str));
    emit notification(tr("Bitshares Client error."));
  }

  return false;
}

void ManageBitShares::waitAndLoadPage()
{
  if(!_server_ready)
    getWebWallet()->onWaitingForServer();

  while(isLaunched() && !_server_ready)
    fc::usleep(fc::milliseconds(500));

  if(isLaunched() && _server_ready)
    getWebWallet()->loadPage();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ManageOtherWallet::ManageOtherWallet(Wallets* walletWeb, const WalletsGui::Server& server)
{
  _wallet_web = walletWeb;
  _server = server;
}

void ManageOtherWallet::loadPage()
{
  getWebWallet()->loadPage();
}
