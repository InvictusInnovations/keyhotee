#include "utils.hpp"

#include "public_key_address.hpp"

#include <bts/profile.hpp>
#include <bts/addressbook/addressbook.hpp>
#include <bts/addressbook/contact.hpp>

#include <QStringList>

namespace Utils
{

QString toString(const fc::ecc::public_key& pk, TContactTextFormatting contactFormatting,
  bts::addressbook::contact* matchingContact /*= nullptr*/)
  {
  assert(pk.valid());

  auto address_book = bts::get_profile()->get_addressbook();
  auto c = address_book->get_contact_by_public_key(pk);
  if (c)
    {
    if(matchingContact != nullptr)
      *matchingContact = *c;

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
QString lTrim(const QString& s)
{
  int n = s.size() ;
  for (int i = 0; i < n; i++) {
    if (!(s.at(i) == ' ')) {
      return s.right(n - i);
    }
  }
  return ""; 
}

} ///namespace Utils

