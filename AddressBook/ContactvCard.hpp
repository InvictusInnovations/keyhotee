#pragma once
#include "vCard/vcard.h"

#include <QList>

class Contact;

/** Converter between Contacts data and vcard data.
 */
class ContactvCard
{
public:
  ContactvCard();
  ContactvCard(const QByteArray& vCardData);
  /** Convert Contact fields to the vCard standard
      \param contact - (in)
      \param vCardData - (out)
  */
  void getvCardData(const Contact& contact, QByteArray* vCardData);
  static bool isValid(const QByteArray& card);

  QString getFirstName() const;
  QString getLastName() const;
  QString getKHID() const;
  QString getPublicKey() const;
  QString getNotes() const;

private:
  vCard _vcard;
};
