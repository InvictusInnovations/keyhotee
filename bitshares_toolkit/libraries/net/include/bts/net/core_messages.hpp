#pragma once

#include <bts/net/config.hpp>

#include <fc/crypto/ripemd160.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/network/ip.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/time.hpp>
#include <fc/variant_object.hpp>
#include <fc/exception/exception.hpp>


#include <vector>

namespace bts { namespace net {

  typedef fc::ripemd160 item_hash_t;
  struct item_id
  {
      uint32_t      item_type;
      item_hash_t   item_hash;

      item_id() {}
      item_id(uint32_t type, const item_hash_t& hash) :
        item_type(type),
        item_hash(hash)
      {}
      bool operator==(const item_id& other) const
      {
        return item_type == other.item_type &&
               item_hash == other.item_hash;
      }
  };

  enum core_message_type_enum
  {
    block_message_type = 1000,//bts::client::block_message_type,
    trx_message_type   = 1001,//bts::client::trx_message_type,
    item_ids_inventory_message_type            = 5001,
    blockchain_item_ids_inventory_message_type = 5002,
    fetch_blockchain_item_ids_message_type     = 5003,
    fetch_items_message_type                   = 5004,
    item_not_available_message_type            = 5005,
    hello_message_type                         = 5006,
    hello_reply_message_type                   = 5007,
    connection_rejected_message_type           = 5008,
    address_request_message_type               = 5009,
    address_message_type                       = 5010,
    closing_connection_message_type            = 5011
  };

  const uint32_t core_protocol_version = BTS_NET_PROTOCOL_VERSION;

  struct item_ids_inventory_message
  {
    static const core_message_type_enum type;

    uint32_t item_type;
    std::vector<item_hash_t> item_hashes_available;

    item_ids_inventory_message() {}
    item_ids_inventory_message(uint32_t item_type, const std::vector<item_hash_t>& item_hashes_available) :
      item_type(item_type),
      item_hashes_available(item_hashes_available)
    {}
  };

  struct blockchain_item_ids_inventory_message
  {
    static const core_message_type_enum type;

    uint32_t total_remaining_item_count;
    uint32_t item_type;
    std::vector<item_hash_t> item_hashes_available;

    blockchain_item_ids_inventory_message() {}
    blockchain_item_ids_inventory_message(uint32_t total_remaining_item_count,
                                          uint32_t item_type,
                                          const std::vector<item_hash_t>& item_hashes_available) :
      total_remaining_item_count(total_remaining_item_count),
      item_type(item_type),
      item_hashes_available(item_hashes_available)
    {}
  };

  struct fetch_blockchain_item_ids_message
  {
    static const core_message_type_enum type;

    uint32_t item_type;
    std::vector<item_hash_t> blockchain_synopsis;

    fetch_blockchain_item_ids_message() {}
    fetch_blockchain_item_ids_message(uint32_t item_type, const std::vector<item_hash_t>& blockchain_synopsis) :
      item_type(item_type),
      blockchain_synopsis(blockchain_synopsis)
    {}
  };

  struct fetch_items_message
  {
    static const core_message_type_enum type;

    uint32_t item_type;
    std::vector<item_hash_t> items_to_fetch;

    fetch_items_message() {}
    fetch_items_message(uint32_t item_type, const std::vector<item_hash_t>& items_to_fetch) :
      item_type(item_type),
      items_to_fetch(items_to_fetch)
    {}
  };

  struct item_not_available_message
  {
    static const core_message_type_enum type;

    item_id requested_item;

    item_not_available_message() {}
    item_not_available_message(const item_id& requested_item) :
      requested_item(requested_item)
    {}
  };

  struct hello_message
  {
    static const core_message_type_enum type;

    std::string        user_agent;
    uint32_t           core_protocol_version;
    fc::ip::address    inbound_address;
    uint16_t           inbound_port;
    uint16_t           outbound_port;
    fc::uint160_t      node_id;
    fc::sha256         chain_id;
    fc::variant_object user_data;

    hello_message() {}
    hello_message(const std::string& user_agent, 
                  uint32_t core_protocol_version, 
                  const fc::ip::address& inbound_address,
                  uint16_t inbound_port,
                  uint16_t outbound_port,
                  const fc::uint160_t& node_id_arg, 
                  const fc::sha256& chain_id_arg,
                  const fc::variant_object& user_data ) :
      user_agent(user_agent),
      core_protocol_version(core_protocol_version),
      inbound_address(inbound_address),
      inbound_port(inbound_port),
      outbound_port(outbound_port),
      node_id(node_id_arg),
      chain_id(chain_id_arg),
      user_data(user_data)
    {}
  };

  struct hello_reply_message
  {
    static const core_message_type_enum type;

    std::string        user_agent;
    uint32_t           core_protocol_version;
    fc::ip::endpoint   remote_endpoint;
    fc::uint160_t      node_id;
    fc::sha256         chain_id;
    fc::variant_object user_data;

    hello_reply_message() {}
    hello_reply_message(const std::string& user_agent, 
                        uint32_t core_protocol_version,
                        fc::ip::endpoint remote_endpoint, 
                        const fc::uint160_t& node_id_arg, 
                        const fc::sha256& chain_id_arg,
                        const fc::variant_object& user_data ) :
      user_agent(user_agent),
      core_protocol_version(core_protocol_version),
      remote_endpoint(remote_endpoint),
      node_id(node_id_arg),
      chain_id(chain_id_arg),
      user_data(user_data)
    {}
  };

  struct connection_rejected_message
  {
    static const core_message_type_enum type;

    std::string      user_agent;
    uint32_t         core_protocol_version;
    fc::ip::endpoint remote_endpoint;
    std::string      rejection_reason;

    connection_rejected_message() {}
    connection_rejected_message(const std::string& user_agent, uint32_t core_protocol_version,
                                const fc::ip::endpoint& remote_endpoint, const std::string& rejection_reason) :
      user_agent(user_agent),
      core_protocol_version(core_protocol_version),
      remote_endpoint(remote_endpoint),
      rejection_reason(rejection_reason)
    {}
  };

  struct address_request_message
  {
    static const core_message_type_enum type;

    address_request_message() {}
  };

  struct address_info
  {
    fc::ip::endpoint   remote_endpoint;
    fc::time_point_sec last_seen_time;

    address_info() {}
    address_info(const fc::ip::endpoint& remote_endpoint, fc::time_point_sec last_seen_time) :
      remote_endpoint(remote_endpoint),
      last_seen_time(last_seen_time)
    {}
  };

  struct address_message
  {
    static const core_message_type_enum type;

    std::vector<address_info> addresses;
  };

  struct closing_connection_message
  {
    static const core_message_type_enum type;

    std::string        reason_for_closing;
    bool               closing_due_to_error;
    fc::oexception     error;

    closing_connection_message() : closing_due_to_error(false) {}
    closing_connection_message(const std::string& reason_for_closing, 
                               bool closing_due_to_error = false, 
                               const fc::oexception& error = fc::oexception()) :
      reason_for_closing(reason_for_closing),
      closing_due_to_error(closing_due_to_error),
      error(error)
    {}
  };

} } // bts::client

FC_REFLECT_ENUM( bts::net::core_message_type_enum, 
                 (block_message_type)
                 (trx_message_type)
                 (item_ids_inventory_message_type)
                 (blockchain_item_ids_inventory_message_type)
                 (fetch_blockchain_item_ids_message_type)
                 (fetch_items_message_type)
                 (item_not_available_message_type)
                 (hello_message_type)
                 (hello_reply_message_type)
                 (connection_rejected_message_type)
                 (address_request_message_type)
                 (address_message_type)
                 (closing_connection_message_type)
                 )
FC_REFLECT( bts::net::item_id, (item_type)(item_hash) )
FC_REFLECT( bts::net::item_ids_inventory_message, (item_type)(item_hashes_available) )
FC_REFLECT( bts::net::blockchain_item_ids_inventory_message, (total_remaining_item_count)(item_type)(item_hashes_available) )
FC_REFLECT( bts::net::fetch_blockchain_item_ids_message, (item_type)(blockchain_synopsis) )
FC_REFLECT( bts::net::fetch_items_message, (item_type)(items_to_fetch) )
FC_REFLECT( bts::net::item_not_available_message, (requested_item) )
FC_REFLECT( bts::net::hello_message, (user_agent)(core_protocol_version)(inbound_address)(inbound_port)(outbound_port)(node_id)(chain_id)(user_data) )
FC_REFLECT( bts::net::hello_reply_message, (user_agent)(core_protocol_version)(remote_endpoint)(node_id)(chain_id)(user_data) )
FC_REFLECT( bts::net::connection_rejected_message, (user_agent)(core_protocol_version)(remote_endpoint)(rejection_reason) )
FC_REFLECT_EMPTY( bts::net::address_request_message )
FC_REFLECT( bts::net::address_info, (remote_endpoint)(last_seen_time) )
FC_REFLECT( bts::net::address_message, (addresses) )
FC_REFLECT( bts::net::closing_connection_message, (reason_for_closing)(closing_due_to_error)(error) )

#include <unordered_map>
#include <fc/crypto/city.hpp>
namespace std
{
    template<>
    struct hash<bts::net::item_id>
    {
       size_t operator()(const bts::net::item_id& item_to_hash) const
       {
          return fc::city_hash_size_t((char*)&item_to_hash, sizeof(item_to_hash));
       }
    };
}
