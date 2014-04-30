#ifndef __GUIUPDATESINK_HPP
#define __GUIUPDATESINK_HPP

#include "ch/mailprocessor.hpp"
#include <fc/optional.hpp>

namespace bts
{
namespace addressbook
{
struct wallet_contact;
} /// namespace addressbook

namespace bitchat
{
struct private_contact_request_message;
struct private_text_message;
} ///namespace bitchat
} /// namespace bts

namespace fc
{
class time_point_sec;
class sha256;
} /// namespace fc

/** Helper callback interface notifying client object about several events.
    This will allow to perform required GUI actions (like refreshing Drafts folder when email
    was saved etc).

    Notifications from this interface should be always called in the GUI thread (usually application
    main thread).
*/
class IGuiUpdateSink
  {
  public:
    typedef bts::bitchat::private_contact_request_message TAuthorizationMessage;
    typedef bts::bitchat::private_text_message            TChatMessage;
    typedef bts::addressbook::wallet_contact              TContact;
    typedef IMailProcessor::TPhysicalMailMessage          TPhysicalMailMessage;
    typedef IMailProcessor::TRecipientPublicKey           TRecipientPublicKey;
    typedef IMailProcessor::TStoredMailMessage            TStoredMailMessage;
    typedef fc::time_point_sec                            TTime;
    typedef bts::bitchat::decrypted_message               TDecryptedMessage;
    typedef fc::optional<fc::sha256>                      TDigest;

  /// Receiving chat messages:
    virtual void OnReceivedChatMessage(const TContact& sender, const TChatMessage& msg,
      const TTime& timeSent) = 0;

  /// Receiving authorization messages:
    virtual void OnReceivedAuthorizationMessage(const TAuthorizationMessage& msg, const TStoredMailMessage& header) = 0;

  /// Receiving mail messages:
    /// Notifies about received mail message.
    virtual void OnReceivedMailMessage(const TStoredMailMessage& msg, const bool spam) = 0;

  /// Receiving unsupported message:
    /// Notifies about received unsupported message.
    virtual void OnReceivedUnsupportedMessage(const TDecryptedMessage& msg) = 0;

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
      const TStoredMailMessage& sentMsg, const TDigest& digest) = 0;
    /// Notifies about message sending end (empty quite).
    virtual void OnMessageSendingEnd() = 0;
    /** Called when there is no matching identity to specified sender PK. Implemented identity
        management feature can lead to such state.
    */
    virtual void OnMissingSenderIdentity(const TRecipientPublicKey& senderId,
      const TPhysicalMailMessage& msg) = 0;

  protected:
    virtual ~IGuiUpdateSink() {}
  };


#endif /// __GUIUPDATESINK_HPP

