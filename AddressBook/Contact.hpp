#pragma once
#include <QDateTime>
#include <QString>
#include <QIcon>
#include <fc/crypto/elliptic.hpp>

#include <bts/addressbook/contact.hpp>


/**
 *  Caches GUI-Specific data associated with the wallet_contact.
 */
class Contact : public bts::addressbook::wallet_contact
{
public:
  Contact(){}
  explicit Contact(const bts::addressbook::wallet_contact&);
  /// Returns true if the identity associated to given contact is owned by current profile's identity.
  bool isOwn() const;
  QString getLabel() const;
  const QIcon& getIcon() const;
  void setIcon(const QIcon& icon);

  //these should probably be moved to bts::wallet_contact API...
  int getAge()const;
  int getRepute()const;
  static int  getAge(const bts::addressbook::contact& id);
  static bool isKeyhoteeFounder(const bts::addressbook::contact& id)
    {
    return getAge(id) == 1;
    }

  bool isKeyhoteeFounder() const
    {
    return isKeyhoteeFounder(*this);
    }

  QString getEmail() const;
  void setEmail(const QString& email);
  QString getPhone() const;
  void setPhone(const QString& phone);
  QString getNotes() const;
  void setNotes(const QString& phone);

  float getMiningEffort() const;
  void setMiningEffort(float miningEffort);

  bool isBlocked() const;

  bool operator<(const Contact& rhs) const { return wallet_index < rhs.wallet_index; }
private:
  /// cache the icon we want to use.
  QIcon icon;
};

typedef std::shared_ptr<Contact> ContactPtr;
