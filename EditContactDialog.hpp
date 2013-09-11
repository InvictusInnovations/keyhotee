#pragma once
#include <QDialog>
#include <memory>
#include <fc/time.hpp>
#include <bts/addressbook/contact.hpp>

namespace Ui { class EditContactDialog; }

class EditContactDialog : public QDialog
{
  public:
     EditContactDialog( QWidget* parent );
     ~EditContactDialog();

     void validateId( const QString& id );
     void lookupId();

     void setContact( const bts::addressbook::contact& con );
     bts::addressbook::contact getContact()const;

  private:
     std::unique_ptr<Ui::EditContactDialog> ui;
     bts::addressbook::contact              _contact;
     bool                                   _complete;
     fc::time_point                         _last_validate;
};
