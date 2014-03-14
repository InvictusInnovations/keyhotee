#include "ContactvCard.hpp"
#include "Contact.hpp"
#include "public_key_address.hpp"

#include <QByteArray>
#include <QIcon>
#include <QImage>
#include <QPixmap>

ContactvCard::ContactvCard(const QByteArray& vCardData)
{  
  vCardList cards = vCard::fromByteArray(vCardData);

  assert (!cards.isEmpty());
  _vcard = cards.takeFirst();
}

void ContactvCard::convert(const Contact& contact, QByteArray* vCardData)
{ 
  vCard vcard;
  vCardProperty name_prop = vCardProperty::createName(contact.first_name.c_str(), 
                                                      contact.last_name.c_str()); 
  vcard.addProperty(name_prop);

  QString formattedName = contact.first_name.c_str() + QString(" ") + contact.last_name.c_str();
  name_prop = vCardProperty::createdFormattedName(formattedName); 
  vcard.addProperty(name_prop);

  name_prop = vCardProperty::createKHID(contact.dac_id_string.c_str());
  vcard.addProperty(name_prop);

  std::string public_key_string = public_key_address(contact.public_key);
  name_prop = vCardProperty::createPublicKey(public_key_string.c_str());
  vcard.addProperty(name_prop);

  name_prop = vCardProperty::createNotes(contact.getNotes());
  vcard.addProperty(name_prop);

  name_prop = vCardProperty::createAvatar(contact.icon_png);
  vcard.addProperty(name_prop);

  *vCardData = vcard.toByteArray();     
}

ContactvCard::ConvertStatus ContactvCard::convert(Contact* contact)
{ 
  contact->first_name       = getFirstName().toStdString();
  contact->last_name        = getLastName().toStdString();
  contact->dac_id_string    = getKHID().toStdString();
  contact->notes            = getNotes().toStdString();
  contact->privacy_setting  = bts::addressbook::secret_contact;
  
  //default icon
  QIcon icon(":/images/user.png");
  //get icon from vCard
  getAvatar(&icon);
  contact->setIcon(icon);

  if (public_key_address::convert(getPublicKey().toStdString(), &contact->public_key) == false)
    return ConvertStatus::PUBLIC_KEY_INVALID;

  return ConvertStatus::SUCCESS;
}

QString ContactvCard::getFirstName() const
{
  vCardProperty name_prop = _vcard.property(VC_NAME);
  QStringList values = name_prop.values();
  return values.at(vCardProperty::Firstname);  
}

QString ContactvCard::getLastName() const
{
  vCardProperty name_prop = _vcard.property(VC_NAME);
  QStringList values = name_prop.values();
  return values.at(vCardProperty::Lastname);  
}

QString ContactvCard::getKHID() const
{
  vCardProperty name_prop = _vcard.property(VC_KHID);
  QString value = name_prop.value();
  return value;
}

QString ContactvCard::getPublicKey() const
{
  vCardProperty name_prop = _vcard.property(VC_KH_PUBLIC_KEY);
  QString value = name_prop.value();
  return value;
}

QString ContactvCard::getNotes() const
{
  vCardProperty name_prop = _vcard.property(VC_NOTE);
  QString value = name_prop.value();
  // "\n" is new line in the vCard standard.
  value.replace("\\n", "\n");
  return value;
}

bool ContactvCard::getAvatar(QIcon* icon)
{
  vCardProperty name_prop = _vcard.property(VC_PHOTO);
  QString value = name_prop.value();
  if (value.isEmpty())
  {
    return false;
  }
  QByteArray byteArray, byteArrayBase64;
  byteArray = value.toLatin1();
  byteArrayBase64 = QByteArray::fromBase64(byteArray);
  QImage image;
  if (! image.loadFromData( byteArrayBase64 ) )
  {
    return false;
  }

  *icon = QIcon(QPixmap::fromImage(image) );

  return true;
}


bool ContactvCard::isValid(const QByteArray& card)
{
  QList<QByteArray> lines = card.split(VC_END_LINE_TOKEN);
  if (lines.size() >= 3)
  {
    if (lines.first() == VC_BEGIN_TOKEN &&
        lines.at(1) == VC_VERSION_2_1_TOKEN)
      return true;
  }

  return false;
}