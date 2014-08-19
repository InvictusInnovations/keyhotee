#include <fc/io/raw.hpp>
#include <fc/reflect/reflect.hpp>
#include <bts/network/stcp_socket.hpp>
#include "bitcoin.hpp"
#include <fc/thread/thread.hpp>
#include <fc/network/ip.hpp>
#include <unordered_map>
#include <fc/log/logger.hpp>
#include <fc/asio.hpp>
#include <fc/crypto/city.hpp>
#include <fc/reflect/variant.hpp>
#include "user_database.hpp"
#include <fc/io/json.hpp>

#include "work_message.hpp"
#include <bts/db/level_map.hpp>
#include <iostream>
#include <fc/crypto/hex.hpp>
#include "momentum.hpp"

#include <boost/exception/all.hpp>
#include <fstream>
#include <stdint.h>

using namespace bts::network;
#define COIN 100000000ll

struct user_stats
  {
  user_record record;
  std::unordered_set<uint64_t> recent_shares;
  };

fc::sha256 Hash(char* b, size_t len)
  {
  auto round1 = fc::sha256::hash(b, len);
  auto round2 = fc::sha256::hash(round1);
  return round2;
  }

uint64_t get_reward(uint64_t block_num)
  {
  int      week = (int)(block_num / 2016);
  uint64_t init_reward = 50 * COIN;
  for (int i = 0; i < week; ++i)
    {
    init_reward *= 95;
    init_reward /= 100;
    }
  return init_reward;
  }

uint64_t       total_paid = 0;
uint64_t       total_earned = 0;
uint64_t       all_shares = 0;
uint64_t       submited = 0;
uint64_t       stale = 0;
uint64_t       total_invalid = 0;
fc::time_point last_window_start = fc::time_point::now();
uint64_t       last_window_start_shares = 0;
double         share_per_min = 0;
bool           server_ok = false;

struct connection_data
  {
  user_record user;
  stcp_socket_ptr sock;
  };

struct config
  {
  config() : fee(0), auto_pay_amount(0), port(4444){}

  double fee;
  double auto_pay_amount;

  std::string host;
  uint16_t port;
  std::string user;
  std::string pass;
  };

FC_REFLECT(config, (host)(port)(user)(pass)(fee)(auto_pay_amount) )


class server
{
public:
  fc::thread*                                           main_thread;
  bts::db::level_map<std::string, user_record>          user_database;
  fc::thread                                            btc_thread;
  fc::bigint                                            share_target;
  uint64_t                                              wallet_balance;
  uint64_t                                              mature_balance;
  std::ofstream                                         payment_log;

  fc::future<void>                                      accept_loop_complete;
  std::unordered_map<fc::ip::endpoint, connection_data> connections;
  fc::tcp_server                                        tcp_serv;

  config                                                conf;

  std::unique_ptr<bitcoin::client>                      bitcoin_client;
  std::unordered_set<uint64_t>                          recent_shares;
  bitcoin::work                                         current_work;

  void load_database()
    {
    auto itr = user_database.begin();
    while (itr.valid() )
      {
      auto r = itr.value();
      total_paid += r.total_paid;
      total_earned += r.total_earned;
      ++itr;
      }
    print_stats();
    }

  void dump_balances()
    {
    auto itr = user_database.begin();
    while (itr.valid() )
      {
      auto k = itr.key();
      auto r = itr.value();
      std::cout << k << ", " << r.valid << ", " << r.invalid << ", " << r.total_earned << ", " << r.total_paid << "\n";
      ++itr;
      }
    }

  void pay_all()
    {
    auto itr = user_database.begin();
    while (itr.valid() )
      {
      auto k = itr.key();
      auto r = itr.value();
      if (r.get_balance() > COIN)
        {
        //pay(k,r.get_balance());
        }
      ++itr;
      }
    print_stats();
    }

  void pay(const std::string& key, uint64_t amount)
    {
    wlog("sending ${amnt} to ${key}", ("amnt", amount)("key", key) );
    try
      {
      std::string trx_id = btc_thread.async( [ = ](){
                                               std::string trx_id = bitcoin_client->sendtoaddress(key, amount);
                                               ilog("sent ${amnt} to ${key} ${trx}", ("amnt", amount)("key", key) ("trx", trx_id) );
                                               return trx_id;
                                               }
                                             ).wait();
      payment_log << std::string(fc::time_point::now()) << ", " << key << ", " << amount << ", " << trx_id;
      total_paid += amount;

      user_record user;
      auto        itr = user_database.find(key);
      if (itr.valid() )
        user = itr.value();
      user.total_paid += amount;
      user_database.store(key, user);
      }
    catch (...)
      {
      wlog("unable to send payment");
      }
    }

  void print_stats()
    {
    fc::time_point now = fc::time_point::now();
    auto           elapsed = (now - last_window_start);
    if (elapsed > fc::seconds(60) )
      {
      uint64_t found_shares = all_shares;
      uint64_t delta = found_shares - last_window_start_shares;

      share_per_min = delta / (elapsed.count() / 60000000.0);

      last_window_start_shares = found_shares;
      last_window_start = now;
      }
    std::cerr << "  wallet: " << (wallet_balance) / double(COIN)
              << "  mature: " << (mature_balance) / double(COIN)
    //     <<"  total_earned: " <<total_earned/double(COIN)
    //     <<"  total_paid: "   <<total_paid/double(COIN)
    //     <<"  total_balance: "<<(total_earned-total_paid)/double(COIN)
    << "  pool: " << (all_shares)
    << "  stale: " << stale
    << "  connections: " << connections.size()
    << "  spm:" << share_per_min
    << " \r";
    }

  server()
    : wallet_balance(0)
    {
    fc::sha256 share_tar;
    memset( (char*)&share_tar, 0xff, sizeof(share_tar) );
    ((char*)&share_tar)[0] = 0x3f;

    share_target = fc::bigint( (char*)&share_tar, sizeof(share_tar) );

    main_thread = &fc::thread::current();

    bitcoin_client.reset(new bitcoin::client(fc::asio::default_io_service() ) );

    user_database.open("users2.db", true);
    payment_log.open("payments.log", std::fstream::out | std::fstream::app);
    }

  ~server()
    {
    close();
    }

  void close()
    {
    ilog("closing connections...");
    try
      {
      tcp_serv.close();
      if (accept_loop_complete.valid() )
        {
        accept_loop_complete.cancel_and_wait();
        }
      }
    catch (const fc::canceled_exception&)
      {
      ilog("expected exception on closing tcp server\n");
      }
    catch (const fc::exception& e)
      {
      wlog("unhandled exception in destructor ${e}", ("e", e.to_detail_string() ));
      }
    catch (...)
      {
      elog("unexpected exception");
      }
    }

  void start_btc_thread()
    {
    btc_thread.async( [ = ](){ bitcoind_thread(); }
                      );
    }

  uint64_t get_next_nonce()
    {
    static uint64_t next_nonce = 0;
    return next_nonce += (1 << 12);
    }

  void bitcoind_thread()
    {
    while (true)
      {
      try
        {
        print_stats();
        bitcoin_client->connect(conf.host + ":3838", conf.user, conf.pass);
        auto latest_work = bitcoin_client->getwork();
        server_ok = true;

        if (latest_work.prev != current_work.prev)
          {
          // NEW BLOCK
          wallet_balance = bitcoin_client->getbalance("*", 1);
          mature_balance = bitcoin_client->getbalance("", 1);
          //       uint64_t    block_num = bitcoin_client->getblockcount();
          //       std::string tar       = bitcoin_client->gettarget();
          //       ilog( "BLOCK NUM ${blocknum}", ("blocknum", block_num) );
          //       ilog( "TARGET NUM ${blocknum}", ("blocknum", tar) );
          /*
                  fc::sha256 tarhash;
                  fc::from_hex( tar, (char*)&tarhash, sizeof(tarhash) );
                  std::reverse( (char*)&tarhash, ((char*)&tarhash)+sizeof(tarhash) );
                  fc::bigint bi( (char*)&tarhash, sizeof(tarhash) );
                  uint64_t  reward = get_reward( block_num );
                  auto shares_per_block = ((share_target / bi).to_int64());
                  current_pps = reward / shares_per_block;
                  current_pps *= 1.0 - conf.fee;
           */
          ilog("NEW BLOCK\n");
          //        ilog( "NEW BLOCK TARGET ${tar} REWARD ${R} PPS ${PPS}  SPP ${SPP}",
          //             ("tar",tarhash)("R",reward/double(COIN))("PPS",current_pps/double(COIN))("SPP",shares_per_block/double(COIN))  );

          update_work(latest_work);
          }
        }
      catch (...)
        {
        server_ok = false;
        fc::usleep(fc::microseconds(1000 * 1000) );
        wlog("server error ${E}", ("E", boost::current_exception_diagnostic_information()) );
        }
      fc::usleep(fc::microseconds(1000 * 500) );
      }
    }

  void submit_work(const bitcoin::work& h)
    {
    if (!btc_thread.is_current() )
      {
      btc_thread.async( [ = ](){ submit_work(h); }
                        ).wait();
      return;
      }
    bitcoin_client->connect(conf.host + ":3838", conf.user, conf.pass);
    bitcoin_client->setwork(h);
    }

  void update_work(const bitcoin::work& latest)
    {
    if (!main_thread->is_current() )
      {
      main_thread->async( [ = ](){ update_work(latest); }
                          );
      return;
      }
    recent_shares.clear();
    current_work = latest;
    for (auto itr = connections.begin(); itr != connections.end(); ++itr)
      {
      fc::async( [ = ](){  send_work(itr->second, current_work); }
                 );
      }
    }

  void send_work(const connection_data& con, const bitcoin::work& latest)
    {
    work_message msg;
    msg.type = 0;
    msg.header = latest;
    msg.header.nonce = get_next_nonce();
    msg.user = con.user;
    msg.pool_spm = share_per_min;
    msg.pool_shares = all_shares;
    msg.pool_earned = wallet_balance;
    msg.mature_balance = mature_balance;

    auto data = fc::raw::pack(msg);
    data.resize(192);
    con.sock->write(data.data(), data.size() );
    }

  void increment_share_count(const std::string& key, bool valid)
    {
    if (!server_ok)
      {
      wlog("Sever ! ok");
      return;
      }

    user_record user;
    auto        itr = user_database.find(key);
    if (itr.valid() )
      user = itr.value();
    if (valid)
      {
      user.valid++;
      }
    else
      {
      user.invalid++;
      total_invalid++;
      }
    user_database.store(key, user);
    }

  void process_connection(connection_data con)
    {
    try
      {
      send_work(con, current_work);

      fc::array<char, 192> packet;
      while (true)
        {
        con.sock->read(packet.data, packet.size() );
        fc::datastream<const char*> ds(packet.data, sizeof(packet) );

        work_message                msg;
        fc::raw::unpack(ds, msg);

        bool                        is_new = recent_shares.insert(fc::city_hash64( (char*)&msg.header, sizeof(msg.header)) ).second;
        if (!is_new)
          continue;

        bool valid = server_ok && verify_share(msg.header);

        increment_share_count(msg.ptsaddr, valid);

        auto itr = user_database.find(msg.ptsaddr);

        if (itr.valid() )
          con.user = itr.value();
        else
          elog("unable to find user in DB");

        send_work(con, current_work);
        }
      }
    catch (const fc::exception&)
      {
      connections.erase(con.sock->get_socket().remote_endpoint() );
      }
    }

  bool verify_share(const bitcoin::work& header)
    {
    if (header.prev != current_work.prev)
      {
      ++stale;
      return false;
      }

    auto result = Hash( (char*)&header, 88);
    std::reverse((char*)&result, ((char*)&result) + sizeof(result) );

    if ( (((unsigned char*)&result)[0] < 0x3f)  )
      {
      auto mid = Hash( (char*)&header, 80);
      if (momentum_verify(mid, header.birthday_a, header.birthday_b) )
        {
        all_shares++;
        if ( (((uint8_t*)&result)[0] == 0x00) &&
             (((uint8_t*)&result)[1] == 0x00) )
          submit_work(header);
        return true;
        }
      }
    return false;
    }

  void accept_connection(const stcp_socket_ptr& s)
    {
    try
      {
      // init DH handshake, TODO: this could yield.. what happens if we exit here before
      // adding s to connections list.
      s->accept();
      ilog("accepted connection from ${ep}",
           ("ep", std::string(s->get_socket().remote_endpoint()) ) );

      connections[s->get_socket().remote_endpoint()].sock = s;
      fc::async( [ = ](){ process_connection(connections[s->get_socket().remote_endpoint()]); }
                 );
      }
    catch (const fc::canceled_exception&)
      {
      ilog("canceled accept operation");
      }
    catch (const fc::exception& e)
      {
      wlog("error accepting connection: ${e}", ("e", e.to_detail_string() ) );
      }
    catch (...)
      {
      elog("unexpected exception");
      }
    }

  void accept_loop() throw()
    {
    try
      {
      while (!accept_loop_complete.canceled() )
        {
        stcp_socket_ptr sock = std::make_shared<stcp_socket>();
        tcp_serv.accept(sock->get_socket() );

        // do the acceptance process async
        fc::async( [ = ](){ accept_connection(sock); }
                   );

        fc::usleep(fc::microseconds(1000) );
        }
      }
    catch (fc::eof_exception&)
      {
      ilog("accept loop eof");
      }
    catch (fc::canceled_exception&)
      {
      ilog("accept loop canceled");
      }
    catch (fc::exception& e)
      {
      elog("tcp server socket threw exception\n ${e}",
           ("e", e.to_detail_string() ) );
      // TODO: notify the server delegate of the error.
      }
    catch (...)
      {
      elog("unexpected exception");
      }
    }
};




int main(int argc, char** argv)
  {
  try
    {
    if (argc < 2)
      {
      std::cerr << "Usage: " << argv[0] << " CONFIG\n";
      return -1;
      }
    server serv;
    serv.conf = fc::json::from_file<config>(argv[1]);

    serv.tcp_serv.listen(serv.conf.port);

    serv.load_database();

    fc::usleep(fc::seconds(1));

    serv.start_btc_thread();

    fc::usleep(fc::seconds(1));

    if (argc > 2)
      {
      wlog("start payments\n");
      serv.pay_all();
      wlog("done with payments\n");
      return 0;
      }
    serv.accept_loop_complete = fc::async( [&](){ serv.accept_loop(); }
                                           );
    serv.accept_loop_complete.wait();

    return 0;
    }
  catch (fc::exception& e)
    {
    std::cerr << e.to_detail_string() << "\n";
    }
  }

