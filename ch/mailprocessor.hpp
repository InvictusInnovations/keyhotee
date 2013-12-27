#ifndef __MAILPROCESSOR_HPP
#define __MAILPROCESSOR_HPP

#include <fc/crypto/elliptic.hpp>

#include <vector>

namespace bts
{
namespace bitchat
{
struct private_email_message;
struct message_header;
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
    /// Type holding a data of message prepared to save/send operation.
    typedef bts::bitchat::private_email_message TPhysicalMailMessage;
    /// Type holding a message data which has been stored in the mail_db.
    typedef bts::bitchat::message_header        TStoredMailMessage;
    typedef bts::addressbook::wallet_identity   TIdentity;
    typedef fc::ecc::public_key                 TRecipientPublicKey;
    typedef std::vector<TRecipientPublicKey>    TRecipientPublicKeys;

    /** Helper callback interface notifying client object about several events.
        This will allow to perform required GUI actions (like refreshing Drafts folder when email
        was saved etc).
    */
    class IUpdateSink
      {
      public:
        typedef IMailProcessor::TPhysicalMailMessage TPhysicalMailMessage;
        typedef IMailProcessor::TRecipientPublicKey  TRecipientPublicKey;
        typedef IMailProcessor::TStoredMailMessage   TStoredMailMessage;

      /// Saving Drafts operation group:
        /// Notifies about start of message save operation
        virtual void OnMessageSaving() = 0;
        /** Notifies about end of message save operation.
            \param msg - message which has been just saved,
            \param overwrittenOne - optional (can be null) message which was replaced by 'msg'.
        */
        virtual void OnMessageSaved(const TStoredMailMessage& msg,
          const TStoredMailMessage* overwrittenOne) = 0;
        
      /// Message Outbox queuing operation group:
        /// Notifies about starting queing process for given number of messages.
        virtual void OnMessageGroupPending(unsigned int count) = 0;
        /** Notifies about queuing a message in the outbox folder.
            \param msg - message just stored in pending folder,
            \param savedDraftMsg - optional (can be nullptr) draft message. If not null, it means that
                         such draft message will be removed from Draft folder in a while.
        */
        virtual void OnMessagePending(const TStoredMailMessage& msg,
          const TStoredMailMessage* savedDraftMsg) = 0;
        /// Notification about message queuing finished.
        virtual void OnMessageGroupPendingEnd() = 0;

      /// Message sending operation group:
        /// Notifies about starting message sending process.
        virtual void OnMessageSendingStart() = 0;
        /// Notifies about end of send operation for given message.
        virtual void OnMessageSent(const TStoredMailMessage& pendingMsg,
          const TStoredMailMessage& sentMsg) = 0;
        /// Notifies about message sending end (empty quite).
        virtual void OnMessageSendingEnd() = 0;
        /** Called when there is no matching identity to specified sender PK. Implemented identity
            management feature can lead to such state.
        */
        virtual void OnMissingSenderIdentity(const TRecipientPublicKey& senderId,
          const TPhysicalMailMessage& msg) = 0;

      protected:
        virtual ~IUpdateSink() {}
      };
    
    /** Allows to schedule given message send to the outbox queue.
        \param senderId      - identity to be used as sender,
        \param msg           - message to be sent,
        \param savedDraftMsg - optional, should be passed not null if one of saved draft messages
                               is about to send.
    */
    virtual void Send(const TIdentity& senderId, const TPhysicalMailMessage& msg,
      const TStoredMailMessage* savedDraftMsg) = 0;

    /** Allows to save given message into Drafts folder in the backend storage.
        \param msgToOverwrite - optional (can be null). If specified given message will be first
                                removed from mail_db and next replaced with new one.
        \param savedMsg       - output, will be filled with saved message representation produced
                                by mail_db.

        \see Send description for other parameter details.
    */
    virtual void Save(const TIdentity& senderId, const TPhysicalMailMessage& msg,
      const TStoredMailMessage* msgToOverwrite, TStoredMailMessage* savedMsg) = 0;

  protected:
    virtual ~IMailProcessor() {}
  };

#endif /// __MAILPROCESSOR_HPP

