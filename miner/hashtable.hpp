#pragma once
#include <fc/crypto/city.hpp>
#include <vector>
#include <array>

#define BIG_TABLE 1
#if BIG_TABLE
const int TABLE_SIZE =  ((1<<26)*1.5);
#else
const int TABLE_SIZE =  (1<<26);
#endif

  class hashtable
  {
     public:
        hashtable() :
            itable(new std::array< std::pair<uint64_t,uint32_t>,TABLE_SIZE >),
            table(*itable)
        {
            reset();
        }
        void reset()
        {
            memset( (char*)table.data(), 0, table.size()*sizeof(std::pair<uint64_t,uint32_t>) );
        }

        uint32_t store( uint64_t key, uint32_t val )
        {
           uint64_t next_key = key;
           auto index = next_key % table.size() ;
              //if matching collision in table, return it
              if( table[index].first == key  )
              {
                  return table[index].second;
              }

           //no collision, add to table
           table[index].first   = key;
           table[index].second  = val;
           return -1;
        }


     private:
        std::array< std::pair<uint64_t,uint32_t>,TABLE_SIZE >*  itable;
        std::array< std::pair<uint64_t,uint32_t>,TABLE_SIZE >&  table;
  };

#if 0
const int TABLE_SIZE =  (1<<26);
  class hashtable
  {
     public:
        hashtable() :
            itable(  new std::array< std::pair<uint64_t,uint32_t> , TABLE_SIZE > ),
            itable2( new std::array< std::pair<uint64_t,uint32_t> , TABLE_SIZE > ),
            table(*itable),
            table2(*itable2)
        {
            reset();
        }
        void reset()
        {
            //table.resize( (1<<26) );
            memset( (char*)table.data(), 0, table.size()*sizeof(std::pair<uint64_t,uint32_t>) );
            //table2.resize( (1<<26) );
            memset( (char*)table2.data(), 0, table2.size()*sizeof(std::pair<uint64_t,uint32_t>) );
        }

        uint32_t store( uint64_t key, uint32_t val )
        {
           uint64_t next_key = key;
           auto index = next_key % table.size() ;
           //if collision
           if ( table[index].first != 0 )
           {
              //if matching collision in first table, return it
              if( table[index].first == key  )
              {
                  return table[index].second;
              }
              //if matching collision in second table, return it
              else if( table2[index].first == key  )
              {
                  return table2[index].second;
              }
              //add to second table
              else
              {
                 table2[index].first   = key;
                 table2[index].second  = val;
                 return -1;
              }
           }
           //no collision, add to first table
           table[index].first   = key;
           table[index].second  = val;
           return -1;
        }


     private:
        std::array< std::pair<uint64_t,uint32_t>,TABLE_SIZE>*  itable;
        std::array< std::pair<uint64_t,uint32_t>,TABLE_SIZE >*  itable2;
        std::array< std::pair<uint64_t,uint32_t>,TABLE_SIZE >&  table;
        std::array< std::pair<uint64_t,uint32_t>,TABLE_SIZE >&  table2;
//        std::vector< std::pair<uint64_t,uint32_t> >  table;
//        std::vector< std::pair<uint64_t,uint32_t> >  table2;
  };
#else
#endif