#include "mailprocessorimpl.hpp"

#include <bts/application.hpp>

#include <atomic>

class TMailProcessor::TOutboxQueue
  {
  public:
    explicit TOutboxQueue(const bts::profile_ptr& profile)
      {
      Outbox = profile->get_pending_db();
      Sent = profile->get_sent_db();
      }

    bool AnyOperationsPending() const;

    /// Returns length of the queue.
    unsigned int GetLength() const;

    void Release()
      {
      assert(AnyOperationsPending() == false);
      delete this;
      }

  private:
    virtual ~TOutboxQueue() {}

  private:
    TMessageDB Outbox;
    TMessageDB Sent;
  };

bool TMailProcessor::TOutboxQueue::AnyOperationsPending() const
  {
  return false;
  }

unsigned int TMailProcessor::TOutboxQueue::GetLength() const
  {
  return 0;
  }

TMailProcessor::TMailProcessor(IUpdateSink& updateSink,
  const bts::profile_ptr& loadedProfile) :
  Sink(updateSink),
  Profile(loadedProfile)
  {
  Drafts = Profile->get_draft_db();
  OutboxQueue = new TOutboxQueue(Profile);
  }

TMailProcessor::~TMailProcessor()
  {
  OutboxQueue->Release();
  }

void TMailProcessor::Send(const TIdentity& senderId, const TPhysicalMailMessage& msg)
  {
  TPhysicalMailMessage msgToSend(msg);
  TRecipientPublicKeys bccList(msg.bcc_list);
  /// \warning Message to be sent must have cleared bcc list.
  msgToSend.bcc_list.clear();

  size_t totalRecipientCount = msgToSend.to_list.size() + msgToSend.cc_list.size() + bccList.size();

  //Sink.OnMessageGroupPending(totalRecipientCount);

  //Sink.OnMessagePending

  auto my_priv_key = Profile->get_keychain().get_identity_key(senderId.dac_id_string);
  auto app = bts::application::instance();

  for(const auto& public_key : msgToSend.to_list)
    app->send_email(msgToSend, public_key, my_priv_key);

  for(const auto& public_key : msgToSend.cc_list)
    app->send_email(msgToSend, public_key, my_priv_key);

  for(const auto& public_key : bccList)
    app->send_email(msgToSend, public_key, my_priv_key);
  }

IMailProcessor::TStoredMailMessage 
TMailProcessor::Save(const TIdentity& senderId, 
                     const TPhysicalMailMessage& sourceMsg,
                     const TStoredMailMessage* msgBeingReplaced)
  {
  Sink.OnMessageSaving();

  TStorableMessage storableMsg;
  PrepareStorableMessage(senderId, sourceMsg, &storableMsg);
  //Modify digest by updating signature time for the draft email.
  //Note that signature time is not true signature time of send, but
  //time when this version of draft email is being saved.
  storableMsg.sig_time = fc::time_point::now();
  TStoredMailMessage savedMsg = Drafts->store_message(storableMsg,msgBeingReplaced);
  Sink.OnMessageSaved(savedMsg, msgBeingReplaced);
  return savedMsg;
  }

void TMailProcessor::PrepareStorableMessage(const TIdentity& senderId,
                                            const TPhysicalMailMessage& sourceMsg, 
                                            TStorableMessage* storableMsg)
  {
  /** It looks for me like another bug in backend. Even decrypted message object can be constructed
      directly by using interface of this class it is not sufficient to successfully store it in
      message_db. All operations performed while sending/receiving email must be performed (except
      transmission of course). So it must be first encrypted and next decrypted to properly fill
      actual decrypted message.
  */
  //DLN Since we're saving message in unencrypted format in dbases, I think it would be better to
  //work with decrypted_message (TStorableMessage) and eliminate most references to TPhysicalMailMesage
  //at GUI level. The only time TPhysicalMailMessage is necessary is when actually sending. So I think
  //it would be better to extract fields from GUI forms to TStorableMessage, then have a function to convert
  //TStorableMessage to TPhysicalMailMessage for sending (opposite of how it is now I think). There's
  //no need to do a full sign/encrypt to save TStorableMessage in database, it just needs to have a unique
  //sig_time (so that the mapping to the message contents is unique even when two email messages have
  //same contents). 
  // For draft messages, this can be achieved by simply setting sig_time
  //"draft save time". I added a line to set the sig_time in the Save function,
  // even though this PrepareStorableMessage currently sets it, 
  // with the idea that we could then eliminate PrepareStorableMessage.
  // Also, we should change the "Date Sent" column in draft mailbox view to "Date Saved".
  // 
  bts::bitchat::decrypted_message msg(sourceMsg);

  auto senderPrivKey = Profile->get_keychain().get_identity_key(senderId.dac_id_string);
  msg.sign(senderPrivKey);
  auto encMsg = msg.encrypt(senderId.public_key);
  encMsg.timestamp = fc::time_point::now();

  encMsg.decrypt(senderPrivKey, *storableMsg);
  }

