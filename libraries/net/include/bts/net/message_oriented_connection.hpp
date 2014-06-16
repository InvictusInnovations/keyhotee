#pragma once
#include <fc/network/tcp_socket.hpp>
#include <bts/net/message.hpp>

namespace bts { namespace net {

  namespace detail { class message_oriented_connection_impl; }

  class message_oriented_connection;

  /** receives incoming messages from a message_oriented_connection object */
  class message_oriented_connection_delegate 
  {
  public:
    virtual void on_message(message_oriented_connection* originating_connection, const message& received_message) = 0;
    virtual void on_connection_closed(message_oriented_connection* originating_connection) = 0;
  };

  /** uses a secure socket to create a connection that reads and writes a stream of `fc::net::message` objects */
  class message_oriented_connection
  {
  public:
    message_oriented_connection(message_oriented_connection_delegate* delegate = nullptr);
    ~message_oriented_connection();
    fc::tcp_socket& get_socket();
    void accept();
    void bind(const fc::ip::endpoint& local_endpoint);
    void connect_to(const fc::ip::endpoint& remote_endpoint);

    void send_message(const message& message_to_send);
    void close_connection();

    uint64_t get_total_bytes_sent() const;
    uint64_t get_total_bytes_received() const;
    fc::time_point get_last_message_sent_time() const;
    fc::time_point get_last_message_received_time() const;
    fc::time_point get_connection_time() const;
  private:
    std::unique_ptr<detail::message_oriented_connection_impl> my;
  };
  typedef std::shared_ptr<message_oriented_connection> message_oriented_connection_ptr;

} } // bts::net
