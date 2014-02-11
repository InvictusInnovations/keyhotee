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
  ~KeyhoteeIDPubKeyWidget();

  enum CurrentState{
    InvalidData,
    OkKeyhoteeID,
    OkPubKey,
    IsStored
  };

  enum ModeWidget{
    AddContact,
    RequestAuthorization
  };

  void onPublicKeyToClipboard();
  void lookupId();
  void lookupPublicKey();

  void setKeyhoteeID(const QString& keyhotee_id);
  void setPublicKey(const QString& public_key_string);
  void showCopyToClipboard(bool visible);
  void setMode(ModeWidget mode = AddContact) {_my_mode = mode;};
  void setEditable(bool editable);

  fc::ecc::public_key getPublicKey();

private:
  Ui::KeyhoteeIDPubKeyWidget *ui;

  void setValid(bool valid);
  bool existContactWithPublicKey (const std::string& public_key_string);

  fc::time_point                          _last_validate;
  Contact                                 _current_contact;
  fc::optional<bts::bitname::name_record> _current_record;
  AddressBookModel*                       _address_book;
  bool                                    _validForm;
  ModeWidget                              _my_mode;

Q_SIGNALS:
  void currentState(CurrentState state);

private slots:
  void keyhoteeIdEdited(const QString& keyhotee_id);
  void publicKeyEdited(const QString& public_key_string);
};

