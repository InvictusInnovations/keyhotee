#include "ContactvCard.hpp"

ContactvCard::ContactvCard()
{
  
}

 void ContactvCard::getvCardData(const Contact* contact, QByteArray* vCardData)
{ 
  vCard vcard;
  vCardProperty name_prop = vCardProperty::createName(contact->first_name.c_str(), 
                                                      contact->last_name.c_str()); 
  vcard.addProperty(name_prop);

  name_prop = vCardProperty::createKHID(contact->dac_id_string.c_str());
  vcard.addProperty(name_prop);

  *vCardData = vcard.toByteArray();  


    // Imagine we've read a byte stream from a data source.
  /*QByteArray in = output;

  // Now we can parse it...
  QList<vCard> vcards = vCard::fromByteArray(in);

  // ...and then we can use it.
  if (!vcards.isEmpty())
  {
      vCard vcard = vcards.takeFirst();

      vCardProperty name_prop = vcard.property(VC_NAME);
//      if (name_prop.isValid())
//      {
          QStringList values = name_prop.values();

          QString firstname = values.at(vCardProperty::Firstname);
          QString lastname = values.at(vCardProperty::Lastname);
//      }
  }*/
}