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
  void getvCardData(const Contact* contact, QByteArray* vCardData);

private:
  vCardList _vCards;
};
