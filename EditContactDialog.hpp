#pragma once
#include <QDialog>
#include <memory>
#include <fc/time.hpp>

namespace Ui { class EditContactDialog; }

class EditContactDialog : public QDialog
{
  public:
     EditContactDialog( QWidget* parent );
     ~EditContactDialog();

     void validateId( const QString& id );
     void lookupId();

  private:
     std::unique_ptr<Ui::EditContactDialog> ui;
     bool                                   _complete;
     fc::time_point                         _last_validate;
};
