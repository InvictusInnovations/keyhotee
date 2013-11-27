#include <fc/crypto/base58.hpp>
#include <fc/crypto/city.hpp>
#include <fc/crypto/elliptic.hpp>

struct public_key_address
{
    fc::ecc::public_key_data key;
    uint32_t                 check;

    bool is_valid()const
    {
       return uint32_t(fc::city_hash64( (char*)&key, sizeof(key) )) == check;
    }
    public_key_address(){}

    public_key_address( const fc::ecc::public_key_data& k )
    {
       key = k;
       check =  uint32_t(fc::city_hash64( (char*)&key, sizeof(key) ));
    }
    public_key_address( const std::string& str )
    {
       std::vector<char> bin = fc::from_base58( str );
       if( bin.size() == 37 )
       {
          memcpy( (char*)&key, bin.data(), 33 );
          memcpy( (char*)&check, bin.data()+33, 4 );
       }
    }
    operator std::string()const
    {
        fc::array<char,37> data;
        memcpy( (char*)&data, (char*)&key, 33 );
        memcpy( ((char*)&data)+33, (char*)&check, 4 );
        return fc::to_base58( (char*)&data, 37 );
    }
};
