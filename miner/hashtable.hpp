#pragma once
#include <fc/crypto/city.hpp>
#include <vector>

  class hashtable
  {
     public:
        hashtable()
        {
            reset();
        }
        void reset()
        {
            table.resize( (1<<26)*1.25 );
            memset( (char*)table.data(), 0, table.size()*sizeof(std::pair<uint64_t,uint32_t>) );
        }

        uint32_t store( uint64_t key, uint32_t val )
        {
           uint64_t next_key = key;
           auto index = next_key % table.size() ;
           while( table[index].first != 0 )
           {
              if( table[index].first == key  )
              {
                  return table[index].second;
              }
              next_key = fc::city_hash64((char*)&next_key,sizeof(next_key));
              index = next_key % table.size() ;
           }
           table[index].first   = key;
           table[index].second  = val;
           return -1;
        }


     private:
        std::vector< std::pair<uint64_t,uint32_t> >  table;
  };
