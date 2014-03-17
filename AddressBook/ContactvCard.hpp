#pragma once
#include "vCard/vcard.h"

class Contact;

/** Converter between Contacts data and vcard data.
 */
class ContactvCard
{
public:
  /// Enum to return convert status
  enum ConvertStatus : int
    {
    PUBLIC_KEY_INVALID  = 1,
    SUCCESS
    };

  ContactvCard(const QByteArray& vCardData);

  /** Convert Contact fields to the vCard standard
      \param contact - (in)
      \param vCardData - (out)
  */
  static void convert(const Contact& contact, QByteArray* vCardData);

  /// Convert vCard data to Contact class
  ConvertStatus convert(Contact* contact);

  /// Check vCard format validation
  static bool isValid(const QByteArray& card);

  QString getFirstName() const;
  QString getLastName() const;
  QString getKHID() const;
  QString getPublicKey() const;
  QString getNotes() const;
  bool getAvatar(QIcon* icon);

private:
  vCard _vcard;
};
