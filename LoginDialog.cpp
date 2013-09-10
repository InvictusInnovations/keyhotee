#include "LoginDialog.hpp"
#include <ui_LoginDialog.h>
#include <bts/application.hpp>

#include <fc/log/logger.hpp>
#include <fc/thread/thread.hpp>

LoginDialog::LoginDialog( QWidget* parent )
:QDialog(parent)
{
    ui.reset( new Ui::LoginDialog() );
    ui->setupUi(this);
    connect( ui->login, &QPushButton::clicked, this, &LoginDialog::onLogin );
    connect( ui->quit, &QPushButton::clicked, this, &LoginDialog::onQuit );
}

LoginDialog::~LoginDialog()
{}


void LoginDialog::onLogin()
{
    try {
       password = ui->password->text().toStdString();
       auto pro = bts::application::instance()->load_profile(password);
       if( pro )
       {
           accept();
       }
    } 
    catch ( const fc::exception& e )
    {
        wlog( "error ${w}", ("w",e.to_detail_string()) );
    }
    ui->password->setText(QString());
    shake();
}
void LoginDialog::shake( )
{
    move( pos() + QPoint(10,0) );
    fc::usleep( fc::microseconds( 50*1000 ) );
    move( pos() + QPoint(-20,0) );
    fc::usleep( fc::microseconds( 50*1000 ) );
    move( pos() + QPoint(20,0) );
    fc::usleep( fc::microseconds( 50*1000 ) );
    move( pos() + QPoint(-10,0) );
}
void LoginDialog::onQuit()
{
    qApp->quit();
    reject();
}
