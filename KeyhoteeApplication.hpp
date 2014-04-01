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
    std::string  getAppName() const;

    std::string  getVersionNumberString() const;

    /// Returns selected (from command line) current profile to load.
    QString getLoadedProfileName() const {return _loaded_profile_name;};
    void setLoadedProfileName(QString loaded_profile_name) {_loaded_profile_name = loaded_profile_name;};

    KeyhoteeMainWindow* getMainWindow() const { return _main_window; }
    void displayLogin();
    void displayProfileWizard();

  private:
    enum TExitStatus
      {
      SUCCESS,
      INTERNAL_ERROR,
      LAST_EXIT_STATUS
      };

    TKeyhoteeApplication(int& argc, char** argv);
    virtual ~TKeyhoteeApplication();

    int  run();

    void onExceptionCaught(const fc::exception& e);
    void onUnknownExceptionCaught();
    void displayFailureInfo(const std::string& detail);

    void startup();

    static void linuxSignalHandler(int);

  /// Overrided from QApplication to catch all exceptions.
    virtual bool notify(QObject * receiver, QEvent * e) override;
  
  /// Class attributes;
  private:
    typedef std::vector<QTemporaryFile*> TTemporaryFileContainer;

    TTemporaryFileContainer _allocated_temps;
    bts::application_ptr    _backend_app;
    QString                 _loaded_profile_name;
    KeyhoteeMainWindow*     _main_window;
    ProfileWizard*          _profile_wizard;
    int                     _exit_status;
    bool                    _last_loaded_profile_name;
    std::string             _keyhoteeVersionNumber;
};



