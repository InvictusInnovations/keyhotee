#ifdef WIN32
#include <Windows.h>
#include <wincon.h>
#endif

#include <bts/application.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <fc/thread/thread.hpp>
#include <fc/io/fstream.hpp>
#include <fc/io/json.hpp>
#include <fc/log/logger.hpp>
#include <fc/reflect/variant.hpp>

#include "profile_wizard/ProfileWizard.hpp"
#include "LoginDialog.hpp"
#include "KeyhoteeMainWindow.hpp"

#include <boost/filesystem/path.hpp>

#include <QApplication>
#include <QStandardPaths>
#include <QTimer>

#include <QFile>
#include <QDebug>

std::string gApplication_name = "Keyhotee";
std::string gProfile_name = "default";
bool gMiningIsPossible = false;


bts::application_config load_config( const std::string& profile_name )
  {
  try 
    {
    auto strDataDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation).toStdWString();
    boost::filesystem::path dataDir(strDataDir);
    boost::filesystem::path profileDataDir(dataDir/profile_name);
    fc::path profileDir(profileDataDir);
    fc::create_directories(profileDir);
    auto config_file  = profileDir / "config.json";
    ilog( "config_file: ${file}", ("file",config_file) );
    if(fc::exists(config_file) == false)
      {
      bts::application_config default_cfg;
      default_cfg.data_dir = profileDir / "data";

      fc::ofstream out( config_file );
      out << fc::json::to_pretty_string( default_cfg );
      }

    auto app_config = fc::json::from_file( config_file ).as<bts::application_config>();
    fc::ofstream out( config_file );
    out << fc::json::to_pretty_string( app_config );
    return app_config;
    }
  FC_RETHROW_EXCEPTIONS( warn, "")
  }


void start_profile_creation_wizard(const bts::application_ptr& btsapp);
void display_login();

void startup( const std::string& profile_name )
{
   auto btsapp     = bts::application::instance();
   auto app_config = load_config( profile_name );
   btsapp->configure( app_config );

   if( btsapp->has_profile() )
   {
      display_login();
   }
   else
   {
      start_profile_creation_wizard(btsapp);
   }
}

int main( int argc, char** argv )
{
  #ifdef WIN32

  bool console_ok = AllocConsole();
  freopen( "CONOUT$", "wb", stdout);
  freopen( "CONOUT$", "wb", stderr);
  printf("testing stdout\n");
  fprintf(stderr,"testing stderr\n");
  #endif
  try {
     QApplication app(argc,argv); 

     app.setOrganizationDomain( "invictus-innovations.com" );
     app.setOrganizationName( "Invictus Innovations, Inc" );

     QFile file(":/index.htm");
     QByteArray dump = file.readAll();
     qDebug() << "contents: " << dump;
     qDebug() << "error status: " << file.error();

     if( argc > 1 ) 
     { 
        gProfile_name = std::string(argv[1]); 
        //if (argc > 2 && !strcmp(argv[2],"beta"))
        //{
        //    gMiningIsPossible = true;
       // }
     }
     gMiningIsPossible = true;

     app.setApplicationName( gApplication_name.c_str() );

     fc::async( [=](){ startup( gProfile_name ); } );

     qApp->connect( qApp, &QApplication::aboutToQuit, [=](){ bts::application::instance()->quit(); } );

     QTimer fc_exec;
     QObject::connect( &fc_exec, &QTimer::timeout, []() { fc::usleep( fc::microseconds(30*1000) ); }  );
     fc_exec.start(5);

     int result = app.exec(); 
     #ifdef WIN32
     fclose(stdout);
     FreeConsole();
     #endif
     return result;
  } 
  catch ( const fc::exception& e )
  {
     elog( "${e}", ("e", e.to_detail_string() ) );
  }
  return -1;
}

void start_profile_creation_wizard( const bts::application_ptr& /*btsapp*/)
{
   // TODO: figure out memory management here..
   auto profile_wizard = new ProfileWizard(nullptr);  
   profile_wizard->resize( QSize( 680, 600 ) );
   profile_wizard->show();
}

void display_main_window()
{
  KeyhoteeMainWindow* main_window = GetKeyhoteeWindow();
  main_window->show();
}

void display_login()
{
    LoginDialog* login_dialog = new LoginDialog();
    login_dialog->connect( login_dialog, &QDialog::accepted,
                    [=](){ 
                        login_dialog->deleteLater();
                        display_main_window(); 
                    } );
    login_dialog->show();
}


