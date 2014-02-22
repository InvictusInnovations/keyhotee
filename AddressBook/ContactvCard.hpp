#pragma once
#include "Contact.hpp"
#include "vcard/vcard.h"

#include <QList>

/**
 *  Converter between Contacts data and vcard data.
 */
class ContactvCard
{
public:
  ContactvCard();
  ContactvCard(QByteArray* vCardData);
  void getvCardData(const Contact* contact, QByteArray* vCardData);
  static bool isValid(const std::vector<char>&);

  QString getFirstName();
  QString getLastName();
  QString getKHID();
  QString getPublicKey();

private:
  vCard _vcard;
};
