#ifndef _BITCOIN_HPP_
#define _BITCOIN_HPP_
#include <boost/asio.hpp>
#include <boost/filesystem/path.hpp>
#include <array>
#include <fc/array.hpp>

namespace bitcoin {
  namespace detail { class client; }

  struct address_info
    {
    bool isvalid;
    std::string address;
    bool ismine;
    std::string account;
    };

  struct server_info
    {
    uint64_t version;
    uint64_t balance;
    uint64_t blocks;
    uint64_t connections;
    std::string proxy;
    bool generate;
    int32_t genproclimit;
    double difficulty;
    double hashespersec;
    bool testnet;
    uint64_t keypoololdest;
    uint64_t paytxfee;
    std::string errors;
    };

  struct work
    {
    uint32_t version;
    fc::array<char, 32> prev;
    fc::array<char, 32> merk;
    uint32_t time;
    uint32_t bits;
    uint32_t nonce;
    uint32_t birthday_a;
    uint32_t birthday_b;
    };


  class client
  {
public:
    client(boost::asio::io_service& ios);
    ~client();

    bool connect(const std::string& host_port, const std::string& user, const std::string& pass);

    std::string gettarget();
    work getwork();
    bool setwork(const work& w);
    std::string backupwallet(const boost::filesystem::path& destination);
    std::string getaccount(const std::string& address);
    std::string getaccountaddress(const std::string& account);
    std::vector<std::string>  getaddressesbyaccount(const std::string& account);
    uint64_t getbalance(const std::string& account = "", uint32_t minconf = 1);
    bool walletpassphrase(const std::string& address, uint64_t amount);
    std::string sendtoaddress(const std::string& address, uint64_t amount);

    //getblockbycount( uint32_t height );
    uint32_t getblockcount();
    uint32_t getblocknumber();
    uint32_t getconnectioncount();
    double getdifficulty();
    bool getgenerate();
    server_info getinfo();

    uint64_t getreceivedbyaddress(const std::string& address, uint32_t minconf = 1);
    uint64_t getreceivedbyaccount(const std::string& account, uint32_t minconf = 1);




    std::string getnewaddress(const std::string& account = "");
    void setaccount(const std::string& address, const std::string& account);
    address_info validateaddress(const std::string& address);


private:
    detail::client* my;

    // create address...
    // query balance...
  };
  }
#include <fc/reflect/reflect.hpp>
FC_REFLECT(bitcoin::work, (version)(prev)(merk)(time)(bits)(nonce)(birthday_a)(birthday_b) )

#endif
