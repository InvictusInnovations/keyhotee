#include "utils.hpp"

#include "public_key_address.hpp"

#include <bts/profile.hpp>
#include <bts/addressbook/addressbook.hpp>
#include <bts/addressbook/contact.hpp>

#include <QStringList>

namespace Utils
{

QString toString(const fc::ecc::public_key& pk, TContactTextFormatting contactFormatting,
  bts::addressbook::contact* matchingContact /*= nullptr*/, bool* isKnownContact /*= nullptr*/)
  {
  assert(pk.valid());

  auto address_book = bts::get_profile()->get_addressbook();
  auto c = address_book->get_contact_by_public_key(pk);
  if (c)
    {
    if(matchingContact != nullptr)
      *matchingContact = *c;
    if(isKnownContact != nullptr)
      *isKnownContact = true;

    switch(contactFormatting)
      {
      case KEYHOTEE_IDENTIFIER:
        return QString(c->dac_id_string.c_str());
      case CONTACT_ALIAS_FULL_NAME:
        return QString(std::string(c->first_name + " " + c->last_name).c_str());
      case FULL_CONTACT_DETAILS:
        return QString(c->get_display_name().c_str());
      default:
        assert(false);
        return QString();
      }
    }
  else
    {
    auto profile = bts::get_profile();
    /// If no contact found try one of registered identities.
    std::vector<bts::addressbook::wallet_identity> identities = profile->identities();
    for(const auto& identity : identities)
      {
      assert(identity.public_key.valid());

      if(identity.public_key == pk)
        {
        if(matchingContact != nullptr)
          *matchingContact = identity;
        if(isKnownContact != nullptr)
          *isKnownContact = true;

        switch(contactFormatting)
          {
          case KEYHOTEE_IDENTIFIER:
            return QString(identity.dac_id_string.c_str());
          case CONTACT_ALIAS_FULL_NAME:
            return QString(std::string(identity.first_name + " " + identity.last_name).c_str());
          case FULL_CONTACT_DETAILS:
            return QString(identity.get_display_name().c_str());
          default:
            assert(false);
            return QString();
          }
        break;
        }
      }

    if(matchingContact != nullptr)
      {
      *matchingContact = bts::addressbook::wallet_contact();
      matchingContact->public_key = pk;
      }

    if(isKnownContact != nullptr)
      *isKnownContact = false;


    /// If code reached this point the publick key is unknown - lets display it as base58
    std::string public_key_string = public_key_address(pk);
    return QString(public_key_string.c_str());
    }
  }

bool matchContact(const fc::ecc::public_key& pk, bts::addressbook::wallet_contact* matchedContact)
{
  assert(pk.valid());

  auto address_book = bts::get_profile()->get_addressbook();
  auto c = address_book->get_contact_by_public_key(pk);
  if (c)
  {
    *matchedContact = *c;
    return true;
  }
  else
  {
    *matchedContact = bts::addressbook::wallet_contact();
    matchedContact->public_key = pk;
  }

  return false;
}

QString 
makeContactListString(const std::vector<fc::ecc::public_key>& key_list, char separator /*= ','*/,
  TContactTextFormatting contactFormatting /* = KEYHOTEE_IDENTIFIER*/)
  {
  QStringList to_list;
  for(const auto& public_key : key_list)
    to_list.append(toString(public_key, contactFormatting));

  return to_list.join(separator);
  }

QString lTrim(QString const& str)
  {
    if (str.size() == 0)
        return str;
    const QChar *s = (const QChar*)str.data();
    if (!s->isSpace() )
        return str;
    int start = 0;
    int end = str.size() - 1;
    while (start<=end && s[start].isSpace())  // skip white space from start
        start++;
    int l = end - start + 1;
    if (l <= 0) {
        QStringDataPtr empty = { QString::Data::allocate(0) };
        return QString(empty);
    }
    return QString(s + start, l);
}

void convertToASCII(const std::string& input, std::string* buffer)
  {
  assert(buffer != nullptr);
  buffer->reserve(input.size());

  for(const auto& c : input)
    {
    unsigned int cCode = c;
    if(cCode > 0x7F)
      {
      /// Non ASCII character
      char numBuffer[64];
      sprintf(numBuffer, "_0x%X_", cCode);
      buffer->append(numBuffer);
      }
    else
      {
      *buffer += c;
      }
    }
  }


} ///namespace Utils

