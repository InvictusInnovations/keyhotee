#pragma once
#include <QWidget>
#include <memory>
#include "Contact.hpp"
#include <fc/time.hpp>
#include <bts/application.hpp>

namespace Ui { class ContactView; }
class AddressBookModel;
class QToolBar;


class ContactView : public QWidget
{
  public:
     enum ContactDisplay { info, chat};
     ContactView( QWidget* parent = nullptr );
     ~ContactView();

     void              setAddressBook( AddressBookModel* address_book );
     AddressBookModel* getAddressBook()const;

     void setContact( const Contact& current_contact);
     Contact getContact()const;

     void onChat();
     void onInfo();
     void onMail();
     void onEdit();
     void onSave();
     void onCancel();
     void onShareContact();
     void onRequestContact();

     void firstNameChanged( const QString& name );
     void lastNameChanged( const QString& name );
     void keyhoteeIdChanged( const QString& name );
     void keyhoteeIdEdited( const QString& name );
     void publicKeyEdited( const QString& public_key_string );
     void onPublicKeyToClipboard ();

     void updateNameLabel();

     void lookupId();
     void lookupPublicKey();

     bool isChatSelected();
     void sendChatMessage();
     void appendChatMessage( const QString& from, const QString& msg, const QDateTime& date_time = QDateTime::currentDateTime() );
     void initTab() const;
     void setAddingNewContact (bool addNew) {_addingNewContact = addNew;};
     bool isAddingNewContact () const {return _addingNewContact;};


  protected:
      bool eventFilter(QObject *obj, QEvent *event);

  private:
     fc::time_point                            _last_validate;
     Contact                                   _current_contact;
     fc::optional<bts::bitname::name_record>   _current_record;
     AddressBookModel*                         _address_book;
     std::unique_ptr<Ui::ContactView>          ui;
     QToolBar*                                 message_tools;
     QAction*                                  send_mail;
     QAction*                                  save_contact;
     QAction*                                  edit_contact;
     QAction*                                  share_contact;
     QAction*                                  request_contact;
     QAction*                                  cancel_edit_contact;
     bool                                      _addingNewContact;
};
