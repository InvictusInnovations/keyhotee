#ifndef __AUTHPROCESSOR_HPP
#define __AUTHPROCESSOR_HPP

#include <bts/profile.hpp>

namespace bts
{
  namespace bitchat
  {
    struct private_contact_request_message;
    struct message_header;
  } ///namespace bitchat
} /// namespace bts

class IAuthProcessor
{
public:
  /// Type holding a message data which has been stored in the auth_db.
  typedef bts::bitchat::private_contact_request_message TRequestMessage;
  /// Type holding a message data which has been stored in the request_db
  typedef bts::bitchat::message_header                  TStoredMessage;
  typedef bts::addressbook::wallet_identity             TCurrIdentity;

  /** Allows to schedule given message send to the outbox queue.
  \param senderId      - identity to be used as sender,
  \param msg           - message to be sent,
  */
  virtual void SendAuth(const TCurrIdentity& senderId, const TRequestMessage& msg) = 0;

  /** Allows to save given message into Authorization folder in the backend storage.
    \param senderId     - identity to be used as sender,
    \param src_msg      - message to be stored,
    \param msg_header   - msg will be removed from request_db and stored in auth_db
  */
  virtual void storeAuthorization(const TCurrIdentity& senderId, 
    const TRequestMessage& src_msg, const TStoredMessage& msg_header) = 0;

protected:
  virtual ~IAuthProcessor() {}
};

#endif /// __AUTHPROCESSOR_HPP

