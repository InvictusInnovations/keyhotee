#pragma once
#include <QWidget>
#include <memory>
#include "Contact.hpp"
#include <fc/time.hpp>
#include <bts/application.hpp>
#include "AddressBook/LineEditReadOnly.hpp"

namespace Ui { class ContactView; }
class AddressBookModel;
class QToolBar;


class ContactView : public QWidget
{
  Q_OBJECT
public:
  enum ContactDisplay { info, chat};
  ContactView(QWidget* parent = nullptr);
  ~ContactView();

  void setAddressBook(AddressBookModel* address_book);
  AddressBookModel* getAddressBook() const;

  void setContact(const Contact& current_contact);
  Contact getContact() const;

  void onChat();
  void onMail();
  void onEdit();
  void onSave();
  void onCancel();
  void onShareContact();
  void onRequestContact();
  void onInfo();

  void onPublicKeyToClipboard();
  void lookupId();
  void lookupPublicKey();

  bool isChatSelected();
  void sendChatMessage();
  void appendChatMessage(const QString& from, const QString& msg, const QDateTime& date_time = QDateTime::currentDateTime() );
  void setAddingNewContact(bool addNew)
    {
    _addingNewContact = addNew;
    }

  bool isAddingNewContact() const
    {
    return _addingNewContact;
    }
  bool CheckSaving();
  void addNewContact ();

Q_SIGNALS:
  void canceledNewContact();
  void savedNewContact(int idxNewContact);

private slots:
  void firstNameChanged(const QString& name);
  void lastNameChanged(const QString& name);
  void keyhoteeIdChanged(const QString& name);
  void keyhoteeIdEdited(const QString& name);
  void publicKeyEdited(const QString& public_key_string);
  void publicKeyChanged(const QString&)
    {
    setModyfied();
    }

  void emailChanged(const QString&)
    {
    setModyfied();
    }

  void phoneChanged(const QString&)
    {
    setModyfied();
    }

  void notesChanged()
    {
    setModyfied();
    }

  void privacyLevelChanged(int)
    {
    setModyfied();
    }

  void currentTabChanged(int index);
  void onSend ();
  void onSliderChanged(int mining_effort)
    {
    _current_contact.setMiningEffort(mining_effort);
    setModyfied();
    }

protected:
  bool eventFilter(QObject *obj, QEvent *event);

private:
  void setModyfied(bool modyfied = true);
  bool isModyfied() const
    {
    return _modyfied;
    }

  bool isEditing() const
    {
    return _editing;
    }

  void setValid(bool valid);
  bool isValid() const
    {
    return _validForm;
    }

  void onIconSearch();
  bool doDataExchange (bool valid);
  bool existContactWithPublicKey (const std::string& public_key_string);
  void keyEdit(bool enable);  
  void setEnabledSaveContact ();

  fc::time_point                          _last_validate;
  Contact                                 _current_contact;
  fc::optional<bts::bitname::name_record> _current_record;
  AddressBookModel*                       _address_book;
  std::unique_ptr<Ui::ContactView>        ui;
  QToolBar*                               message_tools;
  QAction*                                send_mail;
  QAction*                                chat_contact;
  QAction*                                save_contact;
  QAction*                                edit_contact;
  QAction*                                share_contact;
  QAction*                                request_contact;
  QAction*                                cancel_edit_contact;
  QAction*                                separatorToolBar;
  QAction*                                label_createContact;
  bool                                    _addingNewContact;
  bool                                    _modyfied;
  bool                                    _editing;
  bool                                    _validForm;
};
