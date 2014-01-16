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
#include <QSettings>

#include <boost/filesystem/path.hpp>

#include <assert.h>

#ifndef WIN32
  #include <signal.h>
#endif
#include <QTranslator>

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
  /** \warning Use wstring to construct log file name since %TEMP% can point to path containing
      native chars.
  */
  ac.filename = gLogFile.fileName().toStdWString();
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
    app._loaded_profile_name = app.arguments().at(1);
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

TKeyhoteeApplication::TKeyhoteeApplication(int& argc, char** argv) 
:QApplication(argc, argv),
 _loaded_profile_name(""),
 _main_window(nullptr),
 _profile_wizard(nullptr),
 _exit_status(TExitStatus::SUCCESS)
{
  assert(s_Instance == nullptr && "Only one instance allowed at time");
  s_Instance = this;

  _backend_app = bts::application::instance();

  /// \warning use std::wstring to avoid problems related to paths containing native chars.
  auto str_data_dir = QStandardPaths::writableLocation(QStandardPaths::DataLocation).toStdWString();

  fc::path data_dir(str_data_dir);
  fc::path profile_dir( data_dir / "profiles" );
  _backend_app->set_profile_directory( profile_dir );
  _last_loaded_profile_name = false;
}

TKeyhoteeApplication::~TKeyhoteeApplication()
{
  assert(s_Instance == this && "Only one instance allowed at time");
  s_Instance = nullptr;
}

int TKeyhoteeApplication::run()
{
#ifndef WIN32
  signal(SIGSEGV, linuxSignalHandler);
#endif ///WIN32

  try
  {
    setOrganizationDomain("invictus-innovations.com");
    setOrganizationName("Invictus Innovations, Inc");
    setApplicationName(APP_NAME);

    QString locale = QLocale::system().name();
    QTranslator translator;
    translator.load(QString(":/keyhotee_")+locale);
    installTranslator(&translator);

    if(_loaded_profile_name.isEmpty())
    {
      QSettings settings("Invictus Innovations", "Keyhotee");
      _loaded_profile_name = settings.value("last_profile").toString();
      if(!_loaded_profile_name.isEmpty())
        _last_loaded_profile_name = true;
    }

    startup();

    connect(this, &QApplication::aboutToQuit,
      [=]()
      {
        /// Delete the main window. if its object is still alive also mail queue is active what can lead to crash
        delete _main_window;
        _main_window = nullptr;
        _backend_app->quit();
        _backend_app.reset();
      });

    QTimer fc_exec;
    QObject::connect(&fc_exec, &QTimer::timeout, 
                     [](){ fc::usleep(fc::microseconds(30 * 1000) ); }
                    );
    fc_exec.start(30);

    /// increment any QT specific status by last our one to avoid conflicts.
    int rawStatus = (unsigned int)exec();

    if(rawStatus != 0)
      _exit_status = TExitStatus::LAST_EXIT_STATUS + rawStatus;
    else
      _exit_status = TExitStatus::SUCCESS;
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
  LoginDialog* loginDialog = new LoginDialog(*this);
  loginDialog->connect(loginDialog, &QDialog::accepted,
    [ = ]()
    {
      loginDialog->deleteLater();
      displayMainWindow();
    }
    );
  
  if(!_loaded_profile_name.isEmpty())
    if(!loginDialog->isSelectedProfile())
      if(_last_loaded_profile_name)
        QMessageBox::warning(loginDialog, tr("Keyhotee login"), tr("Unable to load last launched profile: ") +
            _loaded_profile_name);
      else
        QMessageBox::warning(loginDialog, tr("Keyhotee login"), tr("Unable to load profile specified on the command line: ") +
            _loaded_profile_name);

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

void TKeyhoteeApplication::startup()
{
  ilog( "." );

  if(_backend_app->has_profile() )
    displayLogin();
  else
    displayProfileWizard();
}

void TKeyhoteeApplication::linuxSignalHandler(int)
{
  // note: to safely throw from a signal handler, you must compile with
  // g++ -fnon-call-exceptions
  throw TSegmentationFaultException();
}


