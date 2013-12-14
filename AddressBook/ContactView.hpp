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
Q_OBJECT
  public:
     enum ContactDisplay { info, chat};
     ContactView( QWidget* parent = nullptr );
     ~ContactView();

     void              setAddressBook( AddressBookModel* address_book );
     AddressBookModel* getAddressBook()const;

     void setContact( const Contact& current_contact);
     Contact getContact()const;

     void onChat();
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
     void publicKeyChanged( const QString&) {setModyfied ();};
     void emailChanged( const QString&) {setModyfied ();};
     void phoneChanged( const QString&) {setModyfied ();};
     void notesChanged() {setModyfied ();};
     void privacyLevelChanged( const QString&) {setModyfied ();};
     void onPublicKeyToClipboard ();

     void updateNameLabel();

     void lookupId();
     void lookupPublicKey();

     bool isChatSelected();
     void sendChatMessage();
     void appendChatMessage( const QString& from, const QString& msg, const QDateTime& date_time = QDateTime::currentDateTime() );
     void setAddingNewContact (bool addNew) {_addingNewContact = addNew;};
     bool isAddingNewContact () const {return _addingNewContact;};
     void keyEdit (bool enable);
     bool CheckSaving();

Q_SIGNALS:
    void canceledAddContact();

  protected:
      bool eventFilter(QObject *obj, QEvent *event);

  private:
      void setModyfied (bool modyfied = true) {_modyfied = modyfied;};
      bool isModyfied () const {return _modyfied;};
      bool isEditing () const {return _editing;};            
      void onTabChanged(int index);
      void setValid (bool valid);
      bool isValid () const {return _validForm;};
      void onIconSearch ();

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
     bool                                      _modyfied;
     bool                                      _editing;
     bool                                      _validForm;
};
