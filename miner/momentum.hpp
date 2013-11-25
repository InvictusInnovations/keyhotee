#pragma once
#include <fc/array.hpp>
#include <fc/io/varint.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/crypto/ripemd160.hpp>
#include <fc/reflect/reflect.hpp>

#define MAX_MOMENTUM_NONCE  (1<<26)

   typedef fc::sha256     pow_seed_type;

   /** 
    *  @return all collisions found in the nonce search space 
    */
   std::vector< std::pair<uint32_t,uint32_t> > momentum_search( pow_seed_type head, int instance = 0 );
   bool momentum_verify( pow_seed_type head, uint32_t a, uint32_t b );


