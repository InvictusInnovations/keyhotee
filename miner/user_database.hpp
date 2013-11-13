#pragma once
#include <fc/reflect/reflect.hpp>

struct user_record
{
    user_record():valid(0),invalid(0),total_earned(0),total_paid(0){}

    uint64_t valid;
    uint64_t invalid;
    int64_t  total_earned;
    int64_t  total_paid;
    int64_t  get_balance()const { return total_earned - total_paid; }
};

FC_REFLECT( user_record, (valid)(invalid)(total_earned)(total_paid) )
