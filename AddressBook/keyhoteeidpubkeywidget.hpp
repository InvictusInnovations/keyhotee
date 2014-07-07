#pragma once
#include <QWidget>
#include "Contact.hpp"
#include <fc/time.hpp>
#include <bts/application.hpp>

namespace Ui {
class KeyhoteeIDPubKeyWidget;
}
class AddressBookModel;

class KeyhoteeIDPubKeyWidget : public QWidget
{
  Q_OBJECT

public:
  explicit KeyhoteeIDPubKeyWidget(QWidget *parent = 0);
  virtual ~KeyhoteeIDPubKeyWidget();

  enum CurrentState{
    InvalidData,
    OkKeyhoteeID,
    OkPubKey,
    IsStored,
    IsIdentity
  };

  enum ModeWidget{
    ShowContact,
    AddContact,
    RequestAuthorization,
    AuthorizationView
  };

  void setAddressBook(AddressBookModel* address_book);
  void setContact(const Contact& current_contact);

  void onPublicKeyToClipboard();
  void lookupId();
  void lookupPublicKey();

  void setKeyhoteeID(const QString& keyhotee_id);
  void setPublicKey(const QString& public_key_string);
  void hideCopyKeysToClipboard();
  void setMode(ModeWidget mode = ShowContact) {_my_mode = mode;};
  void setEditable(bool editable);

  fc::ecc::public_key getPublicKey();
  QString getPublicKeyText();
  QString getKeyhoteeID();
  // override to focused first widget in the KeyhoteeIDPubKeyWidget
  void setFocus(Qt::FocusReason reason);

private:
  bool isLookupThreadActive() const;
  void cancelLookupThread();

  Ui::KeyhoteeIDPubKeyWidget *ui;

  bool existContactWithPublicKey (const std::string& public_key_string);

  Contact                                 _current_contact;
  fc::optional<bts::bitname::name_record> _current_record;
  AddressBookModel*                       _address_book;
  ModeWidget                              _my_mode;
  fc::future<void>                        _lookupThreadState;

Q_SIGNALS:
  void currentState(CurrentState state);

private slots:
  void keyhoteeIdEdited(const QString& keyhotee_id);
  void publicKeyEdited(const QString& public_key_string);
  void on_private_key_to_clipboard_clicked();

  bool event(QEvent *e);
};
