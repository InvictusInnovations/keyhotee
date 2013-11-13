#pragma once
#include "user_database.hpp"
#include "bitcoin.hpp"

struct work_message
{
    work_message()
    :type(0),current_pps(0){}

    uint32_t        type;
    bitcoin::work   header;
    user_record     user;
    uint64_t        current_pps;
    uint64_t        pool_paid;
    uint64_t        pool_earned;
    float           pool_spm;
    float           pool_fee;

    std::string   ptsaddr;
};

enum work_types
{
   SET_WORK,
   INVALID,
   STALE,
   OK
};

#include <fc/reflect/reflect.hpp>
FC_REFLECT( work_message, 
            (type)
            (header)
            (user)
            (current_pps) 
            (pool_paid)
            (pool_earned)
            (pool_spm)
            (pool_fee)
            (ptsaddr) 
          );

