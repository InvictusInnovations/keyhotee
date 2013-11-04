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
     enum ContactDisplay { chat, info, edit };
     ContactView( QWidget* parent = nullptr );
     ~ContactView();

     void              setAddressBook( AddressBookModel* address_book );
     AddressBookModel* getAddressBook()const;

     void setContact( const Contact& current_contact, ContactDisplay contact_display = chat );
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
     void keyhoteeIdEdited( const QString& name );
     void publicKeyEdited( const QString& public_key_string );

     void updateNameLabel();

     void lookupId();
     void lookupPublicKey();

     bool isChatSelected();
     void sendChatMessage();
     void appendChatMessage( const QString& from, const QString& msg, const QDateTime& date_time = QDateTime::currentDateTime() );

  protected:
      bool eventFilter(QObject *obj, QEvent *event);

  private:
     fc::time_point                            _last_validate;
     Contact                                   _current_contact;
     fc::optional<bts::bitname::name_record>   _current_record;
     AddressBookModel*                         _address_book;
     std::unique_ptr<Ui::ContactView>          ui;
};
