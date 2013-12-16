#include "bitcoin.hpp"
#include "base64.hpp"
#include <fc/crypto/hex.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <ostream>
#include <istream>
#include <iostream>
#include <sstream>

#include <exception>
#include <sstream>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <boost/exception/diagnostic_information.hpp>

namespace bitcoin {
  typedef boost::error_info<struct bitcoin_msg_, std::string> bitcoin_msg;

  struct exception : public virtual boost::exception, public virtual std::exception
    {
    const char* what() const throw()
      {
      return "bitcoin::exception";
      }

    virtual void rethrow() const
      {
      BOOST_THROW_EXCEPTION(*this);
      }

    const std::string& message() const
      {
      return *boost::get_error_info<bitcoin_msg>(*this);
      }
    };
    #define COIN 100000000ll

    #define THROW_BITCOIN_EXCEPTION(fmt, ...) \
  do { \
    std::stringstream ss; ss << boost::format(fmt) __VA_ARGS__; \
    BOOST_THROW_EXCEPTION(::bitcoin::exception() << ::bitcoin::bitcoin_msg(ss.str()) ); \
    } \
  while (0)

  using namespace boost::asio::ip;

  namespace detail {
    class client
    {
public:
      client(boost::asio::io_service& i, bitcoin::client* c)
        : ios(i), sock(i), self(c)
                  {}

      boost::property_tree::ptree request(const std::string& json)
        {
        boost::system::error_code error = boost::asio::error::host_not_found;
        sock.close();
        sock.connect(ep, error);
        if (error)
          throw boost::system::system_error(error);

        boost::asio::streambuf request_buf;
        std::ostream           request_info(&request_buf);
        request_info << "POST / HTTP/1.1\n";
        request_info << "Host: 127.0.0.1\n";
        request_info << "Content-Type: application/json-rpc\n";
        request_info << "Authorization: Basic " << b64_password << "\n";
        request_info << "Content-Length: " << json.size() << "\n\n";

        request_info << json;
        // std::cout << std::endl;


        boost::asio::write(sock, request_buf);

        // Read the response status line. The response streambuf will automatically
        //     // grow to accommodate the entire line. The growth may be limited by passing
        //         // a maximum size to the streambuf constructor.
        boost::asio::streambuf response;
        boost::asio::read_until(sock, response, "\n");

        std::istream           response_stream(&response);
        std::string            http_version;
        response_stream >> http_version;
        unsigned int           status_code;
        response_stream >> status_code;
        std::string            status_message;
        std::getline(response_stream, status_message);
        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
          THROW_BITCOIN_EXCEPTION("Invalid Response");
        if (status_code != 200)
          THROW_BITCOIN_EXCEPTION("Response returned with status code %1%", % status_code);
        boost::asio::read_until(sock, response, "\n");

        // Process the response headers.
        std::string header;
        while (std::getline(response_stream, header) && header != "\r")
                                                                                      {}
        //        std::cout << "header:"<< header << "\n";
        //     std::cout << "\n";

        boost::asio::read_until(sock, response, "\n");
        // Write whatever content we already have to output.
        std::stringstream req;
        if (response.size() > 0)
          req << &response;
        //  std::cerr<<req.str()<<"\n";

        using boost::property_tree::ptree;
        ptree             pt;
        std::stringstream rtnss(req.str());
        boost::property_tree::json_parser::read_json(rtnss, pt);
        return pt;
        }

      boost::asio::io_service& ios;
      tcp::socket              sock;
      tcp::endpoint            ep;
      bitcoin::client*         self;

      std::string              user;
      std::string              pass;
      std::string              b64_password;
    };     // detail::client
    } // namespace detail

  client::client(boost::asio::io_service& s)
    {
    my = new detail::client(s, this);
    }
  } // namespace bitcoin

namespace bitcoin {
  client::~client()
    {
    delete my;
    }

  bool client::connect(const std::string& host_port, const std::string& user, const std::string& pass)
    {
    std::string pre_encode = user + ":" + pass;
    my->user = user;
    my->pass = pass;
    my->b64_password = base64_encode( (const unsigned char*)pre_encode.c_str(), pre_encode.size() );

    std::string               host = host_port.substr(0, host_port.find(':') );
    std::string               port = host_port.substr(host.size() + 1);

    tcp::resolver             resolver(my->ios);
    tcp::resolver::query      q(host, port);
    tcp::resolver::iterator   epi = resolver.resolve(q);
    tcp::resolver::iterator   end;

    boost::system::error_code error = boost::asio::error::host_not_found;
    while (error && epi != end)
      {
      my->sock.close();
      my->sock.connect(*epi, error);
      }
    if (error)
      {
      std::cerr << boost::system::system_error(error).what() << std::endl;
      return false;
      }
    my->ep = *epi;

    std::string getinfo = "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"getinfo\", \"params\": [] }";

    //    boost::property_tree::json_parser::write_json( std::cerr, my->request(getinfo) );
    return true;
    }

  server_info client::getinfo()
    {
    std::string                 getinfo = "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"getinfo\", \"params\": [] }";
    boost::property_tree::ptree pt = my->request(getinfo);

    server_info                 si;
    si.version = pt.get<uint64_t   >("result.version");
    si.balance = pt.get<uint64_t   >("result.balance");
    si.blocks = pt.get<uint64_t   >("result.blocks");
    si.connections = pt.get<uint64_t   >("result.connections");
    si.proxy = pt.get<std::string>("result.proxy");
    si.generate = pt.get<bool       >("result.generate");
    si.genproclimit = pt.get<int32_t    >("result.genproclimit");
    si.difficulty = pt.get<double     >("result.difficulty");
    si.hashespersec = pt.get<double     >("result.hashespersec");
    si.testnet = pt.get<bool       >("result.testnet");
    si.keypoololdest = pt.get<uint64_t   >("result.keypoololdest");
    si.paytxfee = pt.get<uint64_t   >("result.paytxfee");
    si.errors = pt.get<std::string>("result.errors");
    return si;
    }

  uint64_t client::getreceivedbyaddress(const std::string& address, uint32_t minconf)
    {
    std::stringstream ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"getreceivedbyaddress\", \"params\": [";
    ss << "\"" << address << "\",";
    ss << minconf;
    ss << "] }";
    return int64_t(my->request(ss.str()).get<double>("result") * 100000000);
    }

  uint64_t client::getreceivedbyaccount(const std::string& account, uint32_t minconf)
    {
    std::stringstream ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"getreceivedbyaccount\", \"params\": [";
    ss << "\"" << account << "\",";
    ss << minconf;
    ss << "] }";
    return int64_t(my->request(ss.str()).get<double>("result") * 100000000);
    }

  uint64_t client::getbalance(const std::string& account, uint32_t minconf)
    {
    std::stringstream ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"getbalance\", \"params\": [";
    ss << "\"" << account << "\",";
    ss << minconf;
    ss << "] }";
    boost::property_tree::json_parser::write_json(std::cerr, my->request(ss.str()) );
    return int64_t(my->request(ss.str()).get<double>("result") * 100000000);
    }

  std::string client::getaccount(const std::string& address)
    {
    std::stringstream ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"getaccount\", \"params\": [";
    ss << "\"" << address << "\"";
    ss << "] }";
    return my->request(ss.str()).get<std::string>("result");
    }

  std::string client::getaccountaddress(const std::string& address)
    {
    std::stringstream ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"getaccountaddress\", \"params\": [";
    ss << "\"" << address << "\"";
    ss << "] }";
    return my->request(ss.str()).get<std::string>("result");
    }

  uint32_t client::getblockcount()
    {
    std::stringstream ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"getblockcount\", \"params\": []}";
    return my->request(ss.str()).get<uint32_t>("result");
    }

  uint32_t client::getconnectioncount()
    {
    std::stringstream ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"getconnectioncount\", \"params\": []}";
    return my->request(ss.str()).get<uint32_t>("result");
    }

  double client::getdifficulty()
    {
    std::stringstream ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"getdifficulty\", \"params\": []}";
    return my->request(ss.str()).get<double>("result");
    }

  bool client::getgenerate()
    {
    std::stringstream ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"getgenerate\", \"params\": []}";
    return my->request(ss.str()).get<std::string>("result") == "true";
    }

  uint32_t client::getblocknumber()
    {
    std::stringstream ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"getblocknumber\", \"params\": []}";
    return my->request(ss.str()).get<uint32_t>("result");
    }

  std::string client::getnewaddress(const std::string& account)
    {
    std::stringstream ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"getnewaddress\", \"params\": [";
    ss << "\"" << account << "\"";
    ss << "] }";
    return my->request(ss.str()).get<std::string>("result");
    }

  address_info client::validateaddress(const std::string& address)
    {
    std::stringstream ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"validateaddress\", \"params\": [";
    ss << "\"" << address << "\"";
    ss << "] }";
    boost::property_tree::ptree pt = my->request(ss.str());
    address_info                ai;
    ai.isvalid = pt.get<std::string>("result.isvalid") == "true";
    ai.ismine = pt.get<std::string>("result.ismine") == "true";
    ai.address = pt.get<std::string>("result.address");
    ai.account = pt.get<std::string>("result.account");
    return ai;
    }

  void client::setaccount(const std::string& address, const std::string& account)
    {
    std::stringstream ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"setaccount\", \"params\": [";
    ss << "\"" << address << "\",";
    ss << "\"" << account << "\"";
    ss << "] }";
    my->request(ss.str());
    }

  std::vector<std::string> client::getaddressesbyaccount(const std::string& account)
    {
    std::stringstream ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"getaddressesbyaccount\", \"params\": [";
    ss << "\"" << account << "\"";
    ss << "] }";
    boost::property_tree::ptree pt = my->request(ss.str());
    boost::property_tree::json_parser::write_json(std::cerr, pt);


    std::vector<std::string>                    addresses;
    boost::property_tree::ptree&                child = pt.get_child("result");
    boost::property_tree::ptree::const_iterator itr = child.begin();
    while (itr != child.end() )
      {
      addresses.push_back(itr->second.data());
      ++itr;
      }


    return addresses;
    }

  std::string client::backupwallet(const boost::filesystem::path& dest)
    {
    std::stringstream ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"backupwallet\", \"params\": [";
    ss << "\"" << dest << "\"";
    ss << "] }";
    return my->request(ss.str()).get<std::string>("error");
    }

  bool client::walletpassphrase(const std::string& pass, uint64_t time)
    {
    std::stringstream ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"walletpassphrase\", \"params\": [";
    ss << "\"" << pass << "\",";
    ss << time;
    ss << "] }";
    auto str = ss.str();
    std::cerr << "\n\n" << str << "\n";

    auto pt = my->request(str);
    boost::property_tree::json_parser::write_json(std::cerr, pt);
    return 0; //pt.get<bool>("result");
    }

  std::string client::sendtoaddress(const std::string& addr, uint64_t amt)
    {
    std::stringstream ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"sendtoaddress\", \"params\": [";
    ss << "\"" << addr << "\",";
    ss << double(amt) / COIN;
    ss << "] }";
    auto str = ss.str();
    std::cerr << "\n\n" << str << "\n";

    auto pt = my->request(str);
    std::cerr << "send " << amt << " TO " << addr << ":\n";
    boost::property_tree::json_parser::write_json(std::cerr, pt);
    return pt.get<std::string>("result");
    }

  void hex_to_bin(const std::string& hexstr, std::vector<char>& bytes)
    {
    bytes.clear();
    for (std::string::size_type i = 0; i < hexstr.size() / 2; ++i)
      {
      std::istringstream iss(hexstr.substr(i * 2, 2));
      unsigned int       n;
      iss >> std::hex >> n;
      bytes.push_back(static_cast<unsigned char>(n));
      }
    }

  work client::getwork()
    {
    std::stringstream            ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"getwork\", \"params\":[]}";
    boost::property_tree::ptree  pt = my->request(ss.str());
    std::vector<char>            bytes;
    boost::property_tree::ptree& child = pt.get_child("result");
    std::string                  data = child.get<std::string>("data");
    hex_to_bin(data, bytes);
    work                         w;
    memcpy( (char*)&w, bytes.data(), sizeof(w) );
    return w;
    }

  std::string client::gettarget()
    {
    std::stringstream            ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"getwork\", \"params\":[]}";
    boost::property_tree::ptree  pt = my->request(ss.str());
    //   boost::property_tree::json_parser::write_json( std::cerr, pt );
    std::vector<char>            bytes;
    boost::property_tree::ptree& child = pt.get_child("result");
    return child.get<std::string>("target");
    }

  bool client::setwork(const work& w)
    {
    std::stringstream ss;
    ss << "{\"jsonrpc\": \"1.0\", \"id\":\"1\", \"method\": \"getwork\", \"params\":[\"";
    ss << std::string(fc::to_hex( (char*)&w, sizeof(w) ) );
    ss << "\"]}";
    //   std::cerr << "result?" << std::string( fc::to_hex( (char*)&w, sizeof(w) ) );
    boost::property_tree::ptree pt = my->request(ss.str());
    boost::property_tree::json_parser::write_json(std::cerr, pt);

    return pt.get<bool>("result");
    }
  }
