#include <fc/thread/thread.hpp>
#include <fc/crypto/ripemd160.hpp>
#include <fc/crypto/sha1.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/crypto/sha512.hpp>
#include <fc/crypto/aes.hpp>

#include <unordered_map>
#include <fc/reflect/variant.hpp>
#include <fc/time.hpp>
#include <algorithm>

#include "hashtable.hpp"
#include "momentum.hpp"
#include <fc/log/logger.hpp>
#include <fc/thread/scoped_lock.hpp>
#include <fc/thread/mutex.hpp>
#include <fc/thread/spin_lock.hpp>
#include <openssl/sha.h>
#include <boost/thread/thread.hpp>
extern "C" {
#include "sphlib-3.0/c/sph_sha2.h"
}

#include <iostream>
	#define MAX_MOMENTUM_NONCE  (1<<26)
	#define SEARCH_SPACE_BITS 50
	#define BIRTHDAYS_PER_HASH 8
   
   volatile bool cancel_search = false;
   uint64_t& get_thread_count()
   {
      static uint64_t thread_count = boost::thread::hardware_concurrency();
      return thread_count;
   }

   std::vector< std::pair<uint32_t,uint32_t> > search( uint32_t offset, hashtable& found, fc::sha256 head )
   {
      std::vector<std::pair<uint32_t,uint32_t> > results;
      results.reserve(16);
      for( uint32_t i = offset*8; !cancel_search && i < MAX_MOMENTUM_NONCE;  )
      {
     //     fc::sha512::encoder enc;
     //     enc.write( (char*)&i, sizeof(i) );
     //     enc.write( (char*)&head, sizeof(head) );
          
          sph_sha512_context context;
          sph_sha512_init( &context );
          sph_sha512(&context, (char*)&i, sizeof(i) );
          sph_sha512(&context, (char*)&head, sizeof(head) );
          fc::sha512 result;
          sph_sha512_close( &context, (char*)&result );
      
     //     auto result = enc.result();
        
          for( uint32_t x = 0; x < 8; ++x )
          {
              uint64_t birthday = result._hash[x] >> 14;
              if( birthday != 0 )
              {
                 uint32_t nonce = i+x;
                 uint32_t cur = found.store( birthday, nonce );
                 if( cur != uint32_t(-1) )
                 {
                     results.push_back( std::make_pair( cur, nonce ) );
                     results.push_back( std::make_pair( nonce, cur ) );
                 }
              }
          }
          i += 8*get_thread_count();
      }
      return results;
   }
    
   std::vector< std::pair<uint32_t,uint32_t> > momentum_search( pow_seed_type head )
   {
      static hashtable found;
      found.reset();
      std::vector< std::pair<uint32_t,uint32_t> > results;
      results.reserve(16);
      fc::spin_lock m;

      static fc::thread       mothreads[32];
      fc::future<std::vector<std::pair<uint32_t,uint32_t>> > done[32];
      
      for( uint32_t i = 0; i < get_thread_count(); ++i )
      {
         done[i]=mothreads[i].async( [&,i](){ return search( i, found, head ); });
      }
      
      for( uint32_t t = 0; t < get_thread_count(); ++t )
      {
          auto r = done[t].wait();
          results.insert( results.end(), r.begin(), r.end() );
      }
      
      return results;
   }

	uint64_t getBirthdayHash(pow_seed_type midHash, uint32_t a)
  {
      uint32_t index = a - a%BIRTHDAYS_PER_HASH;
      char  hash_tmp[sizeof(midHash)+4];
      memcpy( (char*)&hash_tmp[4], (char*)&midHash, sizeof(midHash) );
      memcpy( (char*)&hash_tmp[0], (char*)&index, sizeof(index) );
      
      uint64_t  result_hash[8];
		  SHA512((unsigned char*)hash_tmp, sizeof(hash_tmp), (unsigned char*)&result_hash);
      
      return result_hash[a%BIRTHDAYS_PER_HASH]>>(64-SEARCH_SPACE_BITS);
	}

   bool momentum_verify( pow_seed_type head, uint32_t a, uint32_t b )
   {
      if( a == b ) return false;
      if( a == 0 ) return false;
      if( b == 0 ) return false;
      if( a > MAX_MOMENTUM_NONCE ) return false;
      if( b > MAX_MOMENTUM_NONCE ) return false;

      auto r = (getBirthdayHash(head,a) == getBirthdayHash(head,b));
      return r;
   }

