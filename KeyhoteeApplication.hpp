#ifndef __KEYHOTEEAPPLICATION_HPP
#define __KEYHOTEEAPPLICATION_HPP

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
    static TKeyhoteeApplication* GetInstance();

    /** Builds & starts the app.
        When this function completes it also destroys application object.

        Returned value is application exit status.
    */
    static int  Run(int& argc, char** argv);

    void        DisplayMainWindow();
    void        Quit();

    /// Gives access to the application name.
    const char* GetAppName() const;
    /// Returns selected (from command line) current profile to load.
    const char* GetLoadedProfileName() const;
    /// Tells if currently loaded profile is default one.
    bool        IsDefaultProfileLoaded() const;

    KeyhoteeMainWindow* GetMainWindow() const
      {
      return MainWindow;
      }

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

    int  Run();
    void DisplayLogin();
    void DisplayProfileWizard();

    void OnExceptionCaught(const fc::exception& e);
    void OnUnknownExceptionCaught();
    void DisplayFailureInfo(const std::string& detail);

    bts::application_config LoadConfig();
    void Startup();

    static void LinuxSignalHandler(int);

  /// Overrided from QApplication to catch all exceptions.
    virtual bool notify(QObject * receiver, QEvent * e) override;
  
  /// Class attributes;
  private:
    typedef std::vector<QTemporaryFile*> TTemporaryFileContainer;

    TTemporaryFileContainer AllocatedTemps;
    bts::application_ptr    BackendApp;
    std::string             LoadedProfileName;
    KeyhoteeMainWindow*     MainWindow;
    ProfileWizard*          ProfWizard;
    int                     ExitStatus;
    bool                    DefaultProfileLoaded;
  };

#endif //__KEYHOTEEAPPLICATION_HPP


