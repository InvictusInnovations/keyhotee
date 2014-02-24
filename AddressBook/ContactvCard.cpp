#include "ContactvCard.hpp"
#include "public_key_address.hpp"

ContactvCard::ContactvCard()
{  
}

ContactvCard::ContactvCard(QByteArray* vCardData)
{  
  vCardList cards = vCard::fromByteArray(*vCardData);

  assert (!cards.isEmpty());
  _vcard = cards.takeFirst();
}

 void ContactvCard::getvCardData(const Contact* contact, QByteArray* vCardData)
{ 
  vCard vcard;
  vCardProperty name_prop = vCardProperty::createName(contact->first_name.c_str(), 
                                                      contact->last_name.c_str()); 
  vcard.addProperty(name_prop);

  QString formattedName = contact->first_name.c_str() + QString(" ") + contact->last_name.c_str();
  name_prop = vCardProperty::createdFormattedName(formattedName); 
  vcard.addProperty(name_prop);

  name_prop = vCardProperty::createKHID(contact->dac_id_string.c_str());
  vcard.addProperty(name_prop);

  std::string public_key_string = public_key_address(contact->public_key);
  name_prop = vCardProperty::createPublicKey(public_key_string.c_str());
  vcard.addProperty(name_prop);

  *vCardData = vcard.toByteArray();  
    
}

QString ContactvCard::getFirstName()
{
  vCardProperty name_prop = _vcard.property(VC_NAME);
  QStringList values = name_prop.values();
  return values.at(vCardProperty::Firstname);  
}

QString ContactvCard::getLastName()
{
  vCardProperty name_prop = _vcard.property(VC_NAME);
  QStringList values = name_prop.values();
  return values.at(vCardProperty::Lastname);  
}

QString ContactvCard::getKHID()
{
  vCardProperty name_prop = _vcard.property(VC_KHID);
  QString value = name_prop.value();
  return value;
}

QString ContactvCard::getPublicKey()
{
  vCardProperty name_prop = _vcard.property(VC_KH_PUBLIC_KEY);
  QString value = name_prop.value();
  return value;
}


bool ContactvCard::isValid(const std::vector<char>& card)
{
  QByteArray bytes(card.data());

  QList<QByteArray> lines = bytes.split(VC_END_LINE_TOKEN);
  if (lines.size() >= 3)
  {
    if (lines.first() == VC_BEGIN_TOKEN &&
        lines.at(1) == VC_VERSION_2_1_TOKEN)
      return true;
  }

  return false;
}