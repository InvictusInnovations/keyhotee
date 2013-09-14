#pragma once
#include <QWidget>
#include <memory>
#include "Contact.hpp"
#include <fc/time.hpp>
#include <bts/application.hpp>

namespace Ui { class ContactView; }
class AddressBookModel;
class ContactView : public QWidget
{
  public:
     ContactView( QWidget* parent = nullptr );
     ~ContactView();

     void              setAddressBook( AddressBookModel* addressbook );
     AddressBookModel* getAddressBook()const;

     void setContact( const Contact& current_contact );
     Contact getContact()const;

     void onChat();
     void onInfo();
     void onMail();
     void onEdit();
     void onSave();
     void onCancel();
     void onDelete();

     void firstNameChanged( const QString& name );
     void lastNameChanged( const QString& name );
     void keyhoteeIdChanged( const QString& name );
     void updateNameLabel();

     void lookupId();

     void sendChatMessage();
     void appendChatMessage( const QString& msg );

  protected:
      bool eventFilter(QObject *obj, QEvent *event);

  private:
     bool                                      _complete;
     fc::time_point                            _last_validate;
     Contact                                   _current_contact;
     fc::optional<bts::bitname::name_record>   _current_record;
     AddressBookModel*                         _address_book;
     std::unique_ptr<Ui::ContactView>          ui;
};
