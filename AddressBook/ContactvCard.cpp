#include "ContactvCard.hpp"

ContactvCard::ContactvCard()
{
  
}

void ContactvCard::add(const Contact* contact)
{ 
  //*********** TESTING

   // First we create a valid vCard object...
  vCard vcard;

  vCardProperty name_prop = vCardProperty::createName("John", "Smith");
  //vCardProperty name_prop = vCardProperty::createKHID("John", "Smith");

  vcard.addProperty(name_prop);

  // ...and then we can serialize it and send everywhere.
  QByteArray output = vcard.toByteArray();  

    // Imagine we've read a byte stream from a data source.
  QByteArray in = output;

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
  }
}

QByteArray* ContactvCard::getByteArray()
{
  return new QByteArray();
}