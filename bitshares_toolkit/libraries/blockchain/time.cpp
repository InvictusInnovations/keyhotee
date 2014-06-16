#include <bts/blockchain/time.hpp>

namespace bts { namespace blockchain {
static int32_t adjusted_time_sec = 0;
static int32_t simulated_time    = 0;

void advance_time( int32_t delta_seconds )
{
  adjusted_time_sec += delta_seconds;
}

fc::time_point_sec now()
{
   if( simulated_time ) return fc::time_point() + fc::seconds(simulated_time + adjusted_time_sec);
   return fc::time_point::now() + fc::seconds( adjusted_time_sec );
}

void start_simulated_time( const fc::time_point& t )
{
   simulated_time = t.sec_since_epoch();
   adjusted_time_sec = 0;
}

} } // bts::blockchain
