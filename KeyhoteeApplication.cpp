#include "KeyhoteeApplication.hpp"

#include "LoginDialog.hpp"
#include "KeyhoteeMainWindow.hpp"

#include "profile_wizard/ProfileWizard.hpp"

#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <fc/thread/thread.hpp>
#include <fc/io/fstream.hpp>
#include <fc/io/json.hpp>
#include <fc/log/logger.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/log/file_appender.hpp>

#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTimer>
#include <QTemporaryFile>

#include <boost/filesystem/path.hpp>

#include <assert.h>

#ifndef WIN32
  #include <signal.h>
#endif

static TKeyhoteeApplication* s_Instance = nullptr;

#define APP_NAME "Keyhotee"
#define DEF_PROFILE_NAME "default"

QTemporaryFile gLogFile;

void ConfigureLoggingToTemporaryFile()
{
  //create log file in temporary dir that lasts after application exits
  //DLN later consider moving log to profile, but it's easier to create temporary file for now
  gLogFile.setAutoRemove(false);
  gLogFile.open();
  gLogFile.close();

  //configure logger to also write to log file
  fc::file_appender::config ac;
  ac.filename = gLogFile.fileName().toStdString().c_str();
  ac.truncate = false;
  ac.flush    = true;
  fc::logger::get().add_appender( fc::shared_ptr<fc::file_appender>( new fc::file_appender( fc::variant(ac) ) ) );
}

namespace
{
class TSegmentationFaultException : public std::exception {};
} ///namespace anonymous

TKeyhoteeApplication* TKeyhoteeApplication::getInstance() { return s_Instance; }

int TKeyhoteeApplication::run(int& argc, char** argv)
{
  ConfigureLoggingToTemporaryFile();
  TKeyhoteeApplication app(argc, argv);
  if (argc > 1)
  {
    app._loaded_profile_name = argv[1];
  }

  return app.run();
}

void TKeyhoteeApplication::displayMainWindow()
{
  if(_main_window == nullptr)
  {
    _main_window = new KeyhoteeMainWindow(*this);
    _main_window->show();
    _backend_app->connect_to_network();
  }
}

void TKeyhoteeApplication::quit()
{
  QApplication::quit();
}

std::string TKeyhoteeApplication::getAppName() const
{
  return APP_NAME;
}

std::string TKeyhoteeApplication::getLoadedProfileName() const
{
  return bts::application::instance()->get_profile()->get_name(); //_loaded_profile_name;
}

TKeyhoteeApplication::TKeyhoteeApplication(int& argc, char** argv) 
:QApplication(argc, argv),
 _loaded_profile_name(DEF_PROFILE_NAME),
 _main_window(nullptr),
 _profile_wizard(nullptr),
 _exit_status(TExitStatus::SUCCESS)
{
  assert(s_Instance == nullptr && "Only one instance allowed at time");
  s_Instance = this;

  _backend_app = bts::application::instance();

  /// \warning use std::wstring to avoid problems related to paths containing native chars.
  auto str_data_dir = QStandardPaths::writableLocation(QStandardPaths::DataLocation).toStdWString();
  
  boost::filesystem::path data_dir(str_data_dir);
  fc::path profile_dir( data_dir / "profiles" );
  _backend_app->set_profile_directory( profile_dir );
}

TKeyhoteeApplication::~TKeyhoteeApplication()
{
  assert(s_Instance == this && "Only one instance allowed at time");
  s_Instance = nullptr;
}

int TKeyhoteeApplication::run()
{
  int exitCode = TExitStatus::SUCCESS;
  
#ifndef WIN32
  signal(SIGSEGV, linuxSignalHandler);
#endif ///WIN32

  try
  {
    setOrganizationDomain("invictus-innovations.com");
    setOrganizationName("Invictus Innovations, Inc");
    setApplicationName(APP_NAME);

    startup();

    connect(this, &QApplication::aboutToQuit, [=](){ bts::application::instance()->quit(); });

    QTimer fc_exec;
    QObject::connect(&fc_exec, &QTimer::timeout, 
                     [](){ fc::usleep(fc::microseconds(30 * 1000) ); }
                    );
    fc_exec.start(30);

    /// increment any QT specific status by last our one to avoid conflicts.
    _exit_status = LAST_EXIT_STATUS + (unsigned int)exec();
  }
  catch(const fc::exception& e)
  {
    onExceptionCaught(e);
  }

  catch(...)
  {
    onUnknownExceptionCaught();
  }

  return _exit_status;
}

void TKeyhoteeApplication::displayLogin()
{
  ilog( "." );
  LoginDialog* loginDialog = new LoginDialog();
  loginDialog->connect(loginDialog, &QDialog::accepted,
    [ = ]()
    {
      loginDialog->deleteLater();
      displayMainWindow();
    }
    );
  
  loginDialog->show();
}

void TKeyhoteeApplication::displayProfileWizard()
{
  ilog( "." );
  auto profile_wizard = new ProfileWizard(*this);
  profile_wizard->resize(QSize(680, 600) );
  profile_wizard->show();
}

void TKeyhoteeApplication::onExceptionCaught(const fc::exception& e)
{
  displayFailureInfo(e.to_detail_string());
}

void TKeyhoteeApplication::onUnknownExceptionCaught()
{
  std::string detail("Unknown exception caught");
  displayFailureInfo(detail);
}

void TKeyhoteeApplication::displayFailureInfo(const std::string& detail)
{
  elog("${e}", ("e", detail ) );
  _exit_status = TExitStatus::INTERNAL_ERROR;
  elog("fatal error ${e}",("e",detail));
  _main_window->displayDiagnosticLog();
  quit();
}

bool TKeyhoteeApplication::notify(QObject* receiver, QEvent* e)
{
  try
  {
    return QApplication::notify(receiver, e);
  }
  catch (const fc::exception& e)
  {
    onExceptionCaught(e);
  }
  catch(...)
  {
    onUnknownExceptionCaught();
  }

  return true;
}

bts::application_config TKeyhoteeApplication::loadConfig()
{
  try 
  {
     /*
    fc::create_directories(profile_dir);
    auto config_file = profile_dir / "config.json";

    ilog("config_file: ${file}", ("file", config_file) );
    if (fc::exists(config_file) == false)
    {
      bts::application_config default_cfg;
      default_cfg.data_dir = profile_dir / "data";
      default_cfg.network_port = 0;
      default_cfg.rpc_config.port = 0;
      default_cfg.default_nodes.push_back( fc::ip::endpoint( std::string("162.243.67.4"), 9876 ) );
      
      fc::ofstream out(config_file);
      out << fc::json::to_pretty_string(default_cfg);
    }

    auto app_config = fc::json::from_file(config_file).as<bts::application_config>();
    fc::ofstream out(config_file);
    out << fc::json::to_pretty_string(app_config);
    */
     bts::application_config app_config;
    return app_config;
  }
  FC_RETHROW_EXCEPTIONS(warn, "") 
}

void TKeyhoteeApplication::startup()
{
  ilog( "." );
  _exit_status = TExitStatus::LOAD_CONFIG_FAILURE;

  if(_backend_app->has_profile() )
    displayLogin();
  else
    displayProfileWizard();

  /*
  try 
  {
    auto app_config = loadConfig();
    _exit_status = TExitStatus::BACKEND_CONFIGURATION_FAILURE;
    _backend_app->configure(app_config);
    _exit_status = TExitStatus::SUCCESS;
  }
  catch (fc::exception& e)
  {
    switch(_exit_status)
    {
      case TExitStatus::LOAD_CONFIG_FAILURE:
        elog("Failed to load Keyhotee configuration: ${e}", ("e",e.to_detail_string()));
        break;
      case TExitStatus::BACKEND_CONFIGURATION_FAILURE:
        elog("Failed to configure Keyhotee: ${e}", ("e",e.to_detail_string()));
        break;
      default:
        elog("Failed to startup Keyhotee: ${e}", ("e",e.to_detail_string()));
        break;
    }

    return;
  }
  */

}

void TKeyhoteeApplication::linuxSignalHandler(int)
{
  // note: to safely throw from a signal handler, you must compile with
  // g++ -fnon-call-exceptions
  throw TSegmentationFaultException();
}


