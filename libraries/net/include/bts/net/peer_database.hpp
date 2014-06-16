#pragma once
#include <boost/iterator/iterator_facade.hpp>

#include <fc/network/ip.hpp>
#include <fc/time.hpp>
#include <fc/io/enum_type.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/raw.hpp>

namespace bts { namespace net {

  enum potential_peer_last_connection_disposition
  {
    never_attempted_to_connect,
    last_connection_failed,
    last_connection_rejected,
    last_connection_handshaking_failed,
    last_connection_succeeded
  };

  struct potential_peer_record
  {
    fc::ip::endpoint                  endpoint;
    fc::time_point_sec                last_seen_time;
    fc::enum_type<uint8_t,potential_peer_last_connection_disposition> last_connection_disposition;
    fc::time_point_sec                last_connection_attempt_time;
    uint32_t                          number_of_successful_connection_attempts;
    uint32_t                          number_of_failed_connection_attempts;
    fc::optional<fc::exception>       last_error;

    potential_peer_record()
    :number_of_successful_connection_attempts(0),
    number_of_failed_connection_attempts(0){}

    potential_peer_record(fc::ip::endpoint endpoint,
                          fc::time_point_sec last_seen_time = fc::time_point_sec(),
                          potential_peer_last_connection_disposition last_connection_disposition = never_attempted_to_connect) :
      endpoint(endpoint),
      last_seen_time(last_seen_time),
      last_connection_disposition(last_connection_disposition),
      number_of_successful_connection_attempts(0),
      number_of_failed_connection_attempts(0)
    {}  
  };

  namespace detail
  {
    class peer_database_impl;

    class peer_database_iterator_impl;
    class peer_database_iterator : public boost::iterator_facade<peer_database_iterator, const potential_peer_record, boost::forward_traversal_tag>
    {
    public:
      peer_database_iterator();
      ~peer_database_iterator();
      explicit peer_database_iterator(peer_database_iterator_impl* impl);
      peer_database_iterator( const peer_database_iterator& c );

    private:
      friend class boost::iterator_core_access;
      void increment();
      bool equal(const peer_database_iterator& other) const;
      const potential_peer_record& dereference() const;
    private:      
      std::unique_ptr<peer_database_iterator_impl> my;
    };
  }


  class peer_database
  {
  public:
    peer_database();
    ~peer_database();

    void open(const fc::path& databaseFilename);
    void close();
    void clear();

    void update_entry(const potential_peer_record& updatedRecord);
    potential_peer_record lookup_or_create_entry_for_endpoint(const fc::ip::endpoint& endpointToLookup);

    std::vector<potential_peer_record> get_all()const;

    typedef detail::peer_database_iterator iterator;
    iterator begin();
    iterator end();
    size_t size() const;
  private:
    std::unique_ptr<detail::peer_database_impl> my;
  };

} } // end namespace bts::net

FC_REFLECT_ENUM(bts::net::potential_peer_last_connection_disposition, (never_attempted_to_connect)(last_connection_failed)(last_connection_rejected)(last_connection_handshaking_failed)(last_connection_succeeded))
FC_REFLECT(bts::net::potential_peer_record, (endpoint)(last_seen_time)(last_connection_disposition)(last_connection_attempt_time)(number_of_successful_connection_attempts)(number_of_failed_connection_attempts)(last_error) )
