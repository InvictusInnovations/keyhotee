#include "bitcoin.hpp"
#include <boost/exception/diagnostic_information.hpp>
#include <iostream>
#include <algorithm>
#include "momentum.hpp"
#include <fc/time.hpp>

fc::sha256 Hash( char* b, size_t len )
{
   auto round1 = fc::sha256::hash(b,len);
   auto round2 = fc::sha256::hash(round1);
   return round2;
}

int main( int argc, char** argv )
{
    try {
       if( argc == 1 )
       {
            std::cerr<<"Usage: "<<argv[0]<<" HOST USER PASS\n";
            std::cerr<<"Performing Benchmark...\n";
            fc::sha256 base;
            auto start = fc::time_point::now();

            double total = 0;
            for( uint32_t i = 0; i < 100; ++i )
            {
               base._hash[0]=i;
               base._hash[1]=2*i;
               auto pairs = momentum_search( base );
               total += pairs.size();
               std::cerr<<" "<<total / ((fc::time_point::now()-start).count()/60000000.0)<<" hpm\n";
            }
            auto stop  = fc::time_point::now();
            std::cerr<<"Elapsed: "<<(stop-start).count()/1000000.0<<" seconds\n";
            return -1;
       }
       if( argc < 4 )
       {
            std::cerr<<"Usage: "<<argv[0]<<" HOST USER PASS\n";
            return -1;
       }
    boost::asio::io_service ios;

    std::string host = std::string(argv[1]);

    fc::time_point start = fc::time_point::now();
    uint32_t  total_hashes = 0;
    while( true )
    {
      bitcoin::work       best_work;
      fc::sha256 best;
      memset( (char*)&best, 0xff, sizeof(best) );

      bitcoin::work w;
      {
        bitcoin::client c( ios );
        c.connect( host+":3838", argv[2], argv[3] );
        w = c.getwork();
      }

      for( uint32_t i = 0; i < 1; ++i )
      {
         w.nonce = i;
         
         //auto result = fc::sha256::hash( (char*)&w, 80+8 );
         auto mid = Hash( (char*)&w, 80 );
         auto pairs = momentum_search( mid );
         total_hashes += pairs.size();
         double hpm =  double(total_hashes)/((fc::time_point::now() - start).count()/60000000.0);
         std::cerr<<"HPM: "<<hpm<<"\r";
         for( auto itr = pairs.begin(); itr != pairs.end(); ++itr )
         {
             w.birthday_a = itr->first;
             w.birthday_b = itr->second;
             auto result = Hash( (char*)&w, 88 );
             std::reverse((char*)&result,  ((char*)&result) + sizeof(result) );
             if( result < best ) 
             {
                  best = result;
                  best_work = w;
                  if( (((char*)&best)[0] == 0x00 ) )
                  {
                     bitcoin::client c( ios );
                     c.connect( host+":3838", argv[2], argv[3] );
                     bool r = c.setwork( best_work );
                     std::cerr<<" ACCEPTED: "<<r<<"\n";
                     i = -1;
                     itr = pairs.end();
                     break;
                  }
             }
         }
      }
    }
    } catch ( boost::exception& e )
    {
        std::cerr<< boost::diagnostic_information(e) << std::endl;
    }
}
