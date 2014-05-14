#ifndef __MAILPROCESSOR_HPP
#define __MAILPROCESSOR_HPP

#include <fc/crypto/elliptic.hpp>
#include <bts/profile.hpp>

#include <vector>

namespace bts
{
namespace bitchat
{
struct private_email_message;
struct message_header;
} ///namespace bitchat

} /// namespace bts

/** Client interface to be passed to other parts of the GUI performing a mail operations
    like send, save.
    These operations can be ran in background (ie send: resulting in putting a message first into
    Outbox queue and next into Sent folder).
*/
class IMailProcessor
  {
  public:
    /// Type holding a data of message prepared to save/send operation.
    typedef bts::bitchat::private_email_message   TPhysicalMailMessage;
    /// Type holding a message data which has been stored in the mail_db.
    typedef bts::bitchat::message_header          TStoredMailMessage;
    typedef bts::addressbook::wallet_identity     TIdentity;
    typedef fc::ecc::public_key                   TRecipientPublicKey;
    typedef std::vector<TRecipientPublicKey>      TRecipientPublicKeys;

    enum TMsgType
    {
      Normal,
      Forward,
      Reply
    };

    /** Allows to schedule given message send to the outbox queue.
        \param senderId      - identity to be used as sender,
        \param msg           - message to be sent,
        \param msg_type      - message type: Normal, Forward, Reply
        \param savedDraftMsg - optional, should be passed not null if one of saved draft messages
                               is about to send.
    */
    virtual void Send(const TIdentity& senderId, const TPhysicalMailMessage& msg,
      const TMsgType msg_type, const TStoredMailMessage* savedDraftMsg) = 0;

    /** Allows to save given message into Drafts folder in the backend storage.
        \param msgBeingReplaced - optional (can be null). If specified given message will be first
                                removed from mail_db and next replaced with new one.
        \returns saved message representation produced by mail_db.

        \see Send description for other parameter details.
    */
    virtual TStoredMailMessage Save(const TIdentity& senderId, const TPhysicalMailMessage& sourceMsg,
      const TMsgType msg_type, const TStoredMailMessage* msgBeingReplaced) = 0;

  protected:
    virtual ~IMailProcessor() {}
  };

#endif /// __MAILPROCESSOR_HPP

