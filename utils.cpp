#include "utils.hpp"

#include "public_key_address.hpp"

#include <bts/profile.hpp>
#include <bts/addressbook/contact.hpp>

#include <QStringList>

namespace Utils
{

QString toString(const fc::ecc::public_key& pk, TContactTextFormatting contactFormatting)
  {
  auto address_book = bts::get_profile()->get_addressbook();
  auto c = address_book->get_contact_by_public_key(pk);
  if (c)
    {
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
  else   //display public_key as base58
    {
    std::string public_key_string = public_key_address(pk);
    return QString(public_key_string.c_str());
    }
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

} ///namespace Utils

