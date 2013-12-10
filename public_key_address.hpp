#include <fc/crypto/base58.hpp>
#include <fc/crypto/city.hpp>
#include <fc/crypto/elliptic.hpp>

#include <fc/exception/exception.hpp>

struct public_key_address
{
    fc::ecc::public_key_data key;
    uint32_t                 check;

    public_key_address( const fc::ecc::public_key_data& k )
    {
       key = k;
       check =  uint32_t(fc::city_hash64( (char*)&key, sizeof(key) ));
    }
    
    /** Allows to validate given keyStr before creating a public key object.
        \param keyStr - public key textual representation to be validated

        Returns true if given key is ok, false otherwise.
    */
    static bool is_valid(const std::string& keyStr)
      {
      bool status = false;
      try
        {
        std::vector<char> bin = fc::from_base58(keyStr);
        status = bin.size() == 37;
        }
      catch(const fc::exception&)
        {
        status = false;
        }

      return status;
      }

    /** Allows to construct object of this class from PREVIOUSLY successfully validated key string.
        \see is_valid method for details.
    */
    public_key_address(const std::string& str)
      {
      assert(is_valid(str));
      std::vector<char> bin = fc::from_base58( str );
      assert( bin.size() == 37);
      memcpy( (char*)&key, bin.data(), 33 );
      memcpy( (char*)&check, bin.data()+33, 4 );
      }

    operator std::string()const
    {
        fc::array<char,37> data;
        memcpy( (char*)&data, (char*)&key, 33 );
        memcpy( ((char*)&data)+33, (char*)&check, 4 );
        return fc::to_base58( (char*)&data, 37 );
    }
};
