#pragma once
#include <QWidget>
#include <memory>
#include "Contact.hpp"
#include <fc/time.hpp>
#include <bts/application.hpp>
#include "keyhoteeidpubkeywidget.hpp"

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

  bool isChatSelected();
  void sendChatMessage();
  void checkKeyhoteeIdStatus();
  void appendChatMessage(const QString& from, const QString& msg, const QDateTime& date_time = QDateTime::currentDateTime() );
  void setAddingNewContact(bool addNew);
  bool isAddingNewContact() const { return _addingNewContact; }
  bool isEditing() const { return _editing; }
  bool CheckSaving();
  void addNewContact ();
  void setPublicKey(const QString& public_key_string);
  void setFirstName(const QString& name);
  void setLastName(const QString& name);
  void setNotes(const QString& name);
  void setKHID_or_PublicKey(const QString& khid, const QString& publicKey);

Q_SIGNALS:
  void canceledNewContact();
  void savedNewContact(int idxNewContact);

public slots:
  void checkSendMailButton();
private slots:
  void firstNameChanged(const QString& name);
  void lastNameChanged(const QString& name);
  void emailChanged(const QString&) { setModified(); }
  void phoneChanged(const QString&) { setModified(); }
  void notesChanged()               { setModified(); }
  void privacyLevelChanged(int)     { setModified(); }
  void currentTabChanged(int index);
  void onSend ();
  void onTextChanged();
  void onMiningEffortSliderChanged(int mining_effort);
  void onStateWidget(KeyhoteeIDPubKeyWidget::CurrentState state);
protected:
  bool eventFilter(QObject *obj, QEvent *event);
  void contactEditable(bool enable);
private:
  void setModified(bool modified = true);
  bool isModified() const { return _modified; } 

  void setValid(bool valid);
  bool isValid() const { return _validForm; }

  void onIconSearch();
  void ToDialog();
  bool FromDialog();
  void setEnabledSaveContact();

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
  int static const                        _max_chat_char = 5000;
  bool                                    _addingNewContact;
  bool                                    _modified;
  bool                                    _editing;
  bool                                    _validForm;
};

