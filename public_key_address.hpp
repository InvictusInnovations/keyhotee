#include <fc/crypto/base58.hpp>
#include <fc/crypto/city.hpp>
#include <fc/crypto/elliptic.hpp>

#include <fc/exception/exception.hpp>

#include "utils.hpp"

struct public_key_address
  {
  fc::ecc::public_key_data key;
  uint32_t check;

  public_key_address(const fc::ecc::public_key_data& k)
    {
    key = k;
    check = uint32_t(fc::hash64( (char*)&key, sizeof(key) ));
    }

  /** Allows to validate given keyStr before creating a public key object.
      \param keyStr - public key textual representation to be validated

      Returns true if given key is ok, false otherwise.
   */
  static bool is_valid(const std::string& inputKeyStr, bool* keySemanticallyValid = nullptr)
    {
    /// \warning public key checker/decoder asserts if input string contains national characters
    std::string keyStr;

    Utils::convertToASCII(inputKeyStr, &keyStr);

    bool status = false;
    if(keySemanticallyValid != nullptr)
      *keySemanticallyValid = false;

    try
      {
      std::vector<char> bin = fc::from_base58(keyStr);
      status = bin.size() == 37;
      if(status)
        {
        /** Looks like fc::from_base58 is insufficient - it can accept invalid key values. To be
            sure just try to construct actual public_key object.
        */
        fc::ecc::public_key_data rawData;
        memcpy( (char*)&rawData, bin.data(), 33);
        fc::ecc::public_key checker(rawData);
        
        if (checker.valid() && keySemanticallyValid)
        {
          std::string public_key_string_check = public_key_address(rawData);
          if (public_key_string_check == inputKeyStr)
          {
            *keySemanticallyValid = true;
          }
        }

        return checker.valid();
        }
      }
    catch (const fc::exception&)
      {
      status = false;
      }        

    return status;
    }

  /** Convert public key string to public key data
      Returns false if publicKeyString is not valid
   */
  static bool convert(const std::string& publicKeyString/*in*/, fc::ecc::public_key* publicKeyData/*out*/)
  {
    bool publicKeySemanticallyValid;
    if (is_valid(publicKeyString, &publicKeySemanticallyValid) && publicKeySemanticallyValid)
    {
        public_key_address key_address(publicKeyString);
        *publicKeyData = key_address.key;
        return true;
    }
    return false;
  }

  /** Allows to construct object of this class from PREVIOUSLY successfully validated key string.
      \see is_valid method for details.
   */
  public_key_address(const std::string& str)
    {
    assert(is_valid(str));
    std::vector<char> bin = fc::from_base58(str);
    assert(bin.size() == 37);
    memcpy( (char*)&key, bin.data(), 33);
    memcpy( (char*)&check, bin.data() + 33, 4);
    }

  operator std::string() const
    {
    fc::array<char, 37> data;
    memcpy( (char*)&data, (char*)&key, 33);
    memcpy( ((char*)&data) + 33, (char*)&check, 4);
    return fc::to_base58( (char*)&data, 37);
    }  
  };
