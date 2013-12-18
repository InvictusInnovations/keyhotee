#ifndef __MAILPROCESSOR_HPP
#define __MAILPROCESSOR_HPP

#include <fc/crypto/elliptic.hpp>

#include <vector>

namespace bts
{
namespace bitchat
{
struct private_email_message;
} ///namespace bitchat

namespace addressbook
{
struct wallet_identity;
} ///namespace addressbook

} /// namespace bts

/** Client interface to be passed to other parts of the GUI performing a mail operations
    like send, save.
    These operations can be ran in background (ie send: resulting in putting a message first into
    Outbox queue and next into Sent folder).
*/
class IMailProcessor
  {
  public:
    typedef bts::bitchat::private_email_message TPhysicalMailMessage;
    typedef bts::addressbook::wallet_identity   TIdentity;
    typedef std::vector<fc::ecc::public_key>    TRecipientPublicKeys;
    
    /** Allows to schedule given message send to the outbox queue.
        \param senderId - identity to be used as sender
        \param msg      - message to be sent.
        \param bccList  - optional bcc list (which is not included in the email).
    */
    virtual void Send(const TIdentity& senderId, const TPhysicalMailMessage& msg,
      const TRecipientPublicKeys& bccList) = 0;

    /** Allows to save given message into Drafts folder in the backend storage.
        \see Send description for parameter details.
    */
    virtual void Save(const TIdentity& senderId, const TPhysicalMailMessage& msg,
      const TRecipientPublicKeys& bccList) = 0;

  protected:
    virtual ~IMailProcessor() {}
  };

#endif /// __MAILPROCESSOR_HPP

