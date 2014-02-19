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
  void add(const Contact*);
  QByteArray* getByteArray();

private:
  vCardList _vCards;
};
