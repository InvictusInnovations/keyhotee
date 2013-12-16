#pragma once
#include "user_database.hpp"
#include "bitcoin.hpp"

struct work_message
  {
  work_message()
    : type(0), mature_balance(0), pool_shares(0), pool_earned(0), pool_spm(0), pool_fee(0){}

  uint32_t type;
  bitcoin::work header;
  user_record user;
  uint64_t mature_balance;
  uint64_t pool_shares;
  uint64_t pool_earned;
  float pool_spm;
  float pool_fee;

  std::string ptsaddr;
  };

enum work_types
  {
  SET_WORK,
  INVALID,
  STALE,
  OK
  };

#include <fc/reflect/reflect.hpp>
FC_REFLECT(work_message,
           (type)
           (header)
           (user)
           (mature_balance)
           (pool_shares)
           (pool_earned)
           (pool_spm)
           (pool_fee)
           (ptsaddr)
           );

