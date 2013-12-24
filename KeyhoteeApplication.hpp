#pragma once

#include <bts/application.hpp>

#include <QApplication>

#include <vector>

class KeyhoteeMainWindow;
class ProfileWizard;

class QTemporaryFile;

/** Subclass needed to reimplement 'notify' method and catch unknown exceptions.
    This class also is responsible for temporary files management (removing them) created while
    opening attachement items.
*/
class TKeyhoteeApplication : protected QApplication
{
  Q_OBJECT

  public:
    /// Returns instance of the app - non-null when app object has been built (app is running).
    static TKeyhoteeApplication* getInstance();

    /** Builds & starts the app.
        When this function completes it also destroys application object.

        Returned value is application exit status.
    */
    static int  run(int& argc, char** argv);

    void        displayMainWindow();
    void        quit();

    /// Gives access to the application name.
    std::string getAppName() const;
    /// Returns selected (from command line) current profile to load.
    std::string getLoadedProfileName() const;

    KeyhoteeMainWindow* getMainWindow() const { return _main_window; }

  private:
    enum TExitStatus
      {
      SUCCESS,
      LOAD_CONFIG_FAILURE,
      BACKEND_CONFIGURATION_FAILURE,
      INTERNAL_ERROR,
      LAST_EXIT_STATUS
      };

    TKeyhoteeApplication(int& argc, char** argv);
    virtual ~TKeyhoteeApplication();

    int  run();
    void displayLogin();
    void displayProfileWizard();

    void onExceptionCaught(const fc::exception& e);
    void onUnknownExceptionCaught();
    void displayFailureInfo(const std::string& detail);

    bts::application_config loadConfig();
    void startup();

    static void linuxSignalHandler(int);

  /// Overrided from QApplication to catch all exceptions.
    virtual bool notify(QObject * receiver, QEvent * e) override;
  
  /// Class attributes;
  private:
    typedef std::vector<QTemporaryFile*> TTemporaryFileContainer;

    TTemporaryFileContainer _allocated_temps;
    bts::application_ptr    _backend_app;
    std::string             _loaded_profile_name;
    KeyhoteeMainWindow*     _main_window;
    ProfileWizard*          _profile_wizard;
    int                     _exit_status;
};



