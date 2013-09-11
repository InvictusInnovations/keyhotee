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

#include <QApplication>
#include <QStandardPaths>
#include <QTimer>



bts::application_config load_config( const std::string& profile_name )
{ try {
     auto qdatadir     = QStandardPaths::writableLocation( QStandardPaths::DataLocation );
     auto data_dir     = fc::path( qdatadir.toStdString() ) / profile_name;
     fc::create_directories(data_dir);
     auto config_file  = data_dir / "config.json";
     ilog( "config_file: ${file}", ("file",config_file) );
     if( !fc::exists( config_file ) )
     {
        bts::application_config default_cfg;
        default_cfg.data_dir = data_dir / "data";

        fc::ofstream out( config_file );
        out << fc::json::to_pretty_string( default_cfg );
     }

     auto conf = fc::json::from_file( config_file ).as<bts::application_config>();
     fc::ofstream out( config_file );
     out << fc::json::to_pretty_string( conf );
     return conf;
} FC_RETHROW_EXCEPTIONS( warn, "") }


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
  try {
     QApplication app(argc,argv); 

     app.setOrganizationDomain( "invictus-innovations.com" );
     app.setOrganizationName( "Invictus Innovations, Inc" );
     app.setApplicationName( "Keyhotee" );

     std::string profile_name = "default";

     if( argc > 1 ) 
     { 
        profile_name = std::string(argv[1]); 
     }

     fc::async( [=](){ startup( profile_name ); } );

     qApp->connect( qApp, &QApplication::aboutToQuit, [=](){ bts::application::instance()->quit(); } );

     QTimer fc_exec;
     QObject::connect( &fc_exec, &QTimer::timeout, []() { fc::usleep( fc::microseconds(30*1000) ); }  );
     fc_exec.start(5);

     return app.exec();
  } 
  catch ( const fc::exception& e )
  {
     elog( "${e}", ("e", e.to_detail_string() ) );
  }
  return -1;
}

void start_profile_creation_wizard( const bts::application_ptr& btsapp )
{
   // TODO: figure out memory management here..
   auto pro_wiz = new ProfileWizard(nullptr);  
   pro_wiz->resize( QSize( 640, 525 ) );
   pro_wiz->show();
}

void display_main_window()
{
  KeyhoteeMainWindow* mainwindow = new KeyhoteeMainWindow();
  mainwindow->show();
}

void display_login()
{
    LoginDialog* login = new LoginDialog();
    login->connect( login, &QDialog::accepted,
                    [=](){ 
                        login->deleteLater();
                        display_main_window(); 
                    } );
    login->show();
}


