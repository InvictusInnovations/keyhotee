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

    void onPublicKeyToClipboard();
    void lookupId();
    void lookupPublicKey();

    void setKeyhoteeID(const QString& keyhotee_id);
    void setPublicKey(const QString& public_key_string);

private:
    Ui::KeyhoteeIDPubKeyWidget *ui;

    void setValid(bool valid);
    bool existContactWithPublicKey (const std::string& public_key_string);

    fc::time_point                          _last_validate;
    Contact                                 _current_contact;
    fc::optional<bts::bitname::name_record> _current_record;
    AddressBookModel*                       _address_book;
    bool                                    _validForm;

private slots:
  void keyhoteeIdEdited(const QString& keyhotee_id);
  void publicKeyEdited(const QString& public_key_string);
};

