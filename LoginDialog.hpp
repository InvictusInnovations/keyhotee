#pragma once
#include <QDialog>
#include <memory>

namespace Ui { class LoginDialog; }

class LoginDialog : public QDialog
{
   public:
      LoginDialog( QWidget* parent = nullptr );
      ~LoginDialog();

      void onLogin();
      void onQuit();
      void shake();
      
      std::string password;
   private:
      std::unique_ptr<Ui::LoginDialog> ui;
};
