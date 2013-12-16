#include "bitcoin.hpp"
#include <boost/exception/diagnostic_information.hpp>
#include <iostream>
#include <bts/network/stcp_socket.hpp>
#include <algorithm>
#include "momentum.hpp"
#include "work_message.hpp"
#include <fc/io/raw.hpp>
#include <fc/io/datastream.hpp>
#include <fc/network/resolve.hpp>
#include <fc/thread/thread.hpp>
#include <fc/time.hpp>
#include <fc/log/logger.hpp>
#include <fc/variant.hpp>
#include <boost/thread/thread.hpp>

#define COIN 100000000ll
fc::sha256 Hash(char* b, size_t len)
  {
  auto round1 = fc::sha256::hash(b, len);
  auto round2 = fc::sha256::hash(round1);
  return round2;
  }

extern volatile bool cancel_search;
uint64_t             total_hashes = 0;

uint64_t&            get_thread_count();


void start_work(const bts::network::stcp_socket_ptr& sock, work_message msg, int instance = 0)
  {
  while (!cancel_search)
    {
    auto mid = Hash( (char*)&msg.header, 80);
    auto pairs = momentum_search(mid, instance);

    total_hashes += pairs.size();
    for (auto itr = pairs.begin(); itr != pairs.end(); ++itr)
      {
      msg.header.birthday_a = itr->first;
      msg.header.birthday_b = itr->second;
      auto result = Hash( (char*)&msg.header, 88);
      std::reverse((char*)&result, ((char*)&result) + sizeof(result) );

      if ( (((unsigned char*)&result)[0] < 0x03) )
        {
        std::cout << std::string(fc::time_point::now()) << " " << std::string(result) << "\n";
        auto data = fc::raw::pack(msg);
        data.resize(192);
        sock->write(data.data(), data.size() );
        break;
        }
      }
    fc::usleep(fc::microseconds(100) );
    msg.header.nonce++;
    }
  }

int main(int argc, char** argv)
  {
  try
    {
    if (argc == 1)
      {
      std::cerr << "Usage: " << argv[0] << " HOST PTS_ADDRESS [THREADS=HARDWARE]\n";
      std::cerr << "Performing Benchmark...\n";
      fc::sha256 base;
      auto       start = fc::time_point::now();
      uint32_t   total = 0;
      for (uint32_t i = 0; i < 30; ++i)
        {
        base._hash[0] = i;
        auto pairs = momentum_search(base, 0);
        total += pairs.size();
        std::cerr << "HPM: " << total / ((fc::time_point::now() - start).count() / 60000000.0) << "\r";
        }
      auto stop = fc::time_point::now();
      std::cerr << "HPM: " << total / ((stop - start).count() / 60000000.0) << "\n";
      return -1;
      }
    if (argc < 3)
      {
      std::cerr << "Usage: " << argv[0] << " HOST PTS_ADDRESS\n";
      return -1;
      }
    std::string host = argv[1];
    std::string ptsaddr = argv[2];
    if (argc == 4)
      get_thread_count() = fc::variant(std::string(argv[3]) ).as_uint64();

    std::vector<fc::ip::endpoint> eps = fc::resolve(host, 4444);
    while (true)
      {
      bts::network::stcp_socket_ptr sock = std::make_shared<bts::network::stcp_socket>();
      try
        {
        bool connected = false;
        while (!connected)
          {
          std::cerr << "attempting to connect to " << host << "\n";
          for (uint32_t i = 0; i < eps.size(); ++i)
            {
            try
              {
              ilog("${ep}", ("ep", eps[i]) );
              sock->connect_to(eps[i]);
              connected = true;
              break;
              }
            catch (const fc::exception& e)
              {
              wlog("${w}", ("w", e.to_detail_string() ) );
              }
            }
          if (!connected)
            {
            std::cerr << "Unable to connect\n";
            fc::usleep(fc::seconds(5) );
            sock = std::make_shared<bts::network::stcp_socket>();
            }
          }

        fc::array<char, 192> packet;
        fc::future<void>     search_complete;
        fc::future<void>     search_complete1;

        work_message         msg;
        msg.ptsaddr = ptsaddr;
        auto                 data = fc::raw::pack(msg);
        data.resize(192);

        fc::time_point       start = fc::time_point::now();
        std::cout << "\n";
        total_hashes = 0;
        fc::async( [&](){
                     while (true)
                       {
                       std::cerr << "\r  hpm: " << total_hashes / ((fc::time_point::now() - start).count() / 60000000.0) << "        \r";
                       fc::usleep(fc::seconds(5));
                       }
                     }
                   );

        int count = 0;
        while (true)
          {
          sock->read(packet.data, sizeof(packet) );

          cancel_search = true;
          if (search_complete.valid() )
            search_complete.wait();
          //    if( search_complete1.valid() ) search_complete1.wait();
          cancel_search = false;

          work_message                msg;
          fc::datastream<const char*> ds(packet.data, sizeof(packet) );
          fc::raw::unpack(ds, msg);
          if (count)
            std::cout << "  shares: " << (msg.user.valid)
                      << "  invalid: " << msg.user.invalid
                      << "  pool_shares: " << msg.pool_shares
                      << "  pool_balance: " << (msg.pool_earned / double(COIN))
                      << "  pool_mature: " << (msg.mature_balance / double(COIN))
                      << "  pool_spm: " << (msg.pool_spm)
                      << "  earned(est): " << ((1.0 - msg.pool_fee) * (msg.pool_earned / double(COIN)) * msg.user.valid / double(msg.pool_shares))
                      << "  mature earned(est): " << ((1.0 - msg.pool_fee) * (msg.mature_balance / double(COIN)) * msg.user.valid / double(msg.pool_shares))
                      << "  fee: " << double(msg.pool_fee * 100) << "%"
                      << "  address: " << ptsaddr
                      << "  hpm: " << total_hashes / ((fc::time_point::now() - start).count() / 60000000.0)
                      << "\n";
          else
            std::cout << "  fee: " << double(msg.pool_fee * 100) << "%\n";
          ++count;

          msg.ptsaddr = ptsaddr;
          search_complete = fc::async( [ = ](){ start_work(sock, msg, 0); }
                                       );
          //     search_complete1 = fc::async( [=](){ start_work( sock, msg, 1 ); } );
          }
        }
      catch (fc::exception& e)
        {
        std::cerr << e.to_detail_string() << "\n";
        cancel_search = true;
        }
      }  // while(true)
    }
  catch (boost::exception& e)
    {
    std::cerr << boost::diagnostic_information(e) << std::endl;
    }
  }

