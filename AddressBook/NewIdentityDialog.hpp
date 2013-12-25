#pragma once
#include <QDialog>
#include <memory>

namespace Ui { class NewIdentityDialog; }

class NewIdentityDialog : public QDialog
{
   Q_OBJECT
   public:
      NewIdentityDialog(QWidget* parent = nullptr);
      ~NewIdentityDialog();

      void onUserNameChanged( const QString& name );
      void onSave();

   private:
      std::unique_ptr<Ui::NewIdentityDialog>        ui;
};
