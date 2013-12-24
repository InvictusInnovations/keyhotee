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

   private:
      std::unique_ptr<Ui::NewIdentityDialog>        ui;
};
