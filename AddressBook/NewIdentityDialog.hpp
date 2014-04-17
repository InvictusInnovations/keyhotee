#pragma once
#include <QDialog>

class QString;

namespace Ui { class NewIdentityDialog; }

class NewIdentityDialog : public QDialog
{
   Q_OBJECT
   public:
      NewIdentityDialog(QWidget* parent = nullptr);
      ~NewIdentityDialog();

  signals:
    void identityadded();

  private:
    void onUserNameChanged(const QString& name);
    void onKey(const QString& name);
    void onSave();

   private:
      Ui::NewIdentityDialog*        ui;
};
