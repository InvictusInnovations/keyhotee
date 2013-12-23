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

TKeyhoteeApplication* TKeyhoteeApplication::GetInstance() { return s_Instance; }

int TKeyhoteeApplication::Run(int& argc, char** argv)
  {
  ConfigureLoggingToTemporaryFile();
  TKeyhoteeApplication app(argc, argv);
  if (argc > 1)
    {
    app.LoadedProfileName = argv[1];
    app.DefaultProfileLoaded = app.LoadedProfileName == DEF_PROFILE_NAME;
    }

  return app.Run();
  }

void TKeyhoteeApplication::DisplayMainWindow()
  {
  if(MainWindow == nullptr)
    {
    MainWindow = new KeyhoteeMainWindow(*this);
    MainWindow->show();
    BackendApp->connect_to_network();
    }
  }

void TKeyhoteeApplication::Quit()
  {
  quit();
  }

const char* TKeyhoteeApplication::GetAppName() const
  {
  return APP_NAME;
  }

const char* TKeyhoteeApplication::GetLoadedProfileName() const
  {
  return LoadedProfileName.c_str();
  }

bool TKeyhoteeApplication::IsDefaultProfileLoaded() const
  {
  return DefaultProfileLoaded;
  }

TKeyhoteeApplication::TKeyhoteeApplication(int& argc, char** argv) :
  QApplication(argc, argv),
  LoadedProfileName(DEF_PROFILE_NAME),
  MainWindow(nullptr),
  ProfWizard(nullptr),
  ExitStatus(TExitStatus::SUCCESS),
  DefaultProfileLoaded(true)
  {
  assert(s_Instance == nullptr && "Only one instance allowed at time");
  s_Instance = this;

  BackendApp = bts::application::instance();
  }

TKeyhoteeApplication::~TKeyhoteeApplication()
  {
  assert(s_Instance == this && "Only one instance allowed at time");
  s_Instance = nullptr;
  }

int TKeyhoteeApplication::Run()
  {
  int exitCode = TExitStatus::SUCCESS;
  
#ifndef WIN32
  signal(SIGSEGV, LinuxSignalHandler);
#endif ///WIN32

  try
    {
    setOrganizationDomain("invictus-innovations.com");
    setOrganizationName("Invictus Innovations, Inc");
    setApplicationName(APP_NAME);

    fc::async( [ = ](){ Startup(); }
              );

    connect(this, &QApplication::aboutToQuit, [ = ](){ bts::application::instance()->quit(); }
                  );

    QTimer fc_exec;
    QObject::connect(&fc_exec, &QTimer::timeout, [] () { fc::usleep(fc::microseconds(30 * 1000) ); }
                      );
    fc_exec.start(5);
    /// increment any QT specific status by last our one to avoid conflicts.
    ExitStatus = LAST_EXIT_STATUS + (unsigned int)exec();
    }
  catch(const fc::exception& e)
    {
    OnExceptionCaught(e);
    }

  catch(...)
    {
    OnUnknownExceptionCaught();
    }

  return ExitStatus;
  }

void TKeyhoteeApplication::DisplayLogin()
  {
  LoginDialog* loginDialog = new LoginDialog();
  loginDialog->connect(loginDialog, &QDialog::accepted,
    [ = ]()
      {
      loginDialog->deleteLater();
      DisplayMainWindow();
      }
    );
  
  loginDialog->show();
  }

void TKeyhoteeApplication::DisplayProfileWizard()
  {
  auto profile_wizard = new ProfileWizard(*this);
  profile_wizard->resize(QSize(680, 600) );
  profile_wizard->show();
  }

void TKeyhoteeApplication::OnExceptionCaught(const fc::exception& e)
  {
  DisplayFailureInfo(e.to_detail_string());
  }

void TKeyhoteeApplication::OnUnknownExceptionCaught()
  {
  std::string detail("Unknown exception caught");
  DisplayFailureInfo(detail);
  }

void TKeyhoteeApplication::DisplayFailureInfo(const std::string& detail)
  {
  elog("${e}", ("e", detail ) );
  ExitStatus = TExitStatus::INTERNAL_ERROR;
  QMessageBox::critical(nullptr, tr("Application internal error"),
    tr("Application encountered internal error.\nError details: ") + QString(detail.c_str()));
  Quit();
  }

bool TKeyhoteeApplication::notify(QObject* receiver, QEvent* e)
  {
  try
    {
    return QApplication::notify(receiver, e);
    }
  catch (const fc::exception& e)
    {
    OnExceptionCaught(e);
    }
  catch(...)
    {
    OnUnknownExceptionCaught();
    }

  return true;
  }

bts::application_config TKeyhoteeApplication::LoadConfig()
  {
  try 
    {
    /// \warning use stdwstring to avoid problems related to paths containing native chars.
    auto strDataDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation).toStdWString();
    boost::filesystem::path dataDir(strDataDir);
    boost::filesystem::path profileDataDir(dataDir / LoadedProfileName);
    fc::path profileDir(profileDataDir);
    fc::create_directories(profileDir);
    auto config_file = profileDir / "config.json";
    ilog("config_file: ${file}", ("file", config_file) );
    if (fc::exists(config_file) == false)
      {
      bts::application_config default_cfg;
      default_cfg.data_dir = profileDir / "data";
      default_cfg.network_port = 0;
      default_cfg.rpc_config.port = 0;
      default_cfg.default_nodes.push_back( fc::ip::endpoint( std::string("162.243.67.4"), 9876 ) );
      
      fc::ofstream out(config_file);
      out << fc::json::to_pretty_string(default_cfg);
      }

    auto app_config = fc::json::from_file(config_file).as<bts::application_config>();
    fc::ofstream out(config_file);
    out << fc::json::to_pretty_string(app_config);
    return app_config;
    }
  FC_RETHROW_EXCEPTIONS(warn, "") 
  }

void TKeyhoteeApplication::Startup()
  {
  ExitStatus = TExitStatus::LOAD_CONFIG_FAILURE;

  try 
    {
    auto app_config = LoadConfig();
    ExitStatus = TExitStatus::BACKEND_CONFIGURATION_FAILURE;
    BackendApp->configure(app_config);
    ExitStatus = TExitStatus::SUCCESS;
    }
  catch (fc::exception& e)
    {
    switch(ExitStatus)
      {
      case TExitStatus::LOAD_CONFIG_FAILURE:
        elog("Failed to load Keyhotee configuration: ${e}", ("e",e.to_detail_string()));
        break;
      case TExitStatus::BACKEND_CONFIGURATION_FAILURE:
        elog("Failed to configure Keyhotee: ${e}", ("e",e.to_detail_string()));
        break;
      default:
        elog("Failed to Startup Keyhotee: ${e}", ("e",e.to_detail_string()));
        break;
      }

    return;
    }

  if(BackendApp->has_profile() )
    DisplayLogin();
  else
    DisplayProfileWizard();
  }

void TKeyhoteeApplication::LinuxSignalHandler(int)
  {
  // note: to safely throw from a signal handler, you must compile with
  // g++ -fnon-call-exceptions
  throw TSegmentationFaultException();
  }


