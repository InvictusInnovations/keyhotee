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

void TMailProcessor::Send(const TIdentity& senderId, const TPhysicalMailMessage& msg,
  const TRecipientPublicKeys& bccList)
  {
  size_t totalRecipientCount = msg.to_list.size() + msg.cc_list.size() + bccList.size();

  //Sink.OnMessageGroupPending(totalRecipientCount);

  //Sink.OnMessagePending

  auto my_priv_key = Profile->get_keychain().get_identity_key(senderId.dac_id_string);
  auto app = bts::application::instance();

  for(const auto& public_key : msg.to_list)
    app->send_email(msg, public_key, my_priv_key);

  for(const auto& public_key : msg.cc_list)
    app->send_email(msg, public_key, my_priv_key);

  for(const auto& public_key : bccList)
    app->send_email(msg, public_key, my_priv_key);
  }

void TMailProcessor::Save(const TIdentity& senderId, const TPhysicalMailMessage& sourceMsg,
  const TRecipientPublicKeys& bccList, const TStoredMailMessage* msgToOverwrite,
  TStoredMailMessage* savedMsg)
  {
  assert(savedMsg != nullptr);

  Sink.OnMessageSaving();

  TStorableMessage storableMsg;
  PrepareStorableMessage(senderId, sourceMsg, &storableMsg);

  *savedMsg = Drafts->store(storableMsg);

  /** FIXME - block for another bug in backend. It is impossible to uniquely identify message_header
      object.
      https://github.com/InvictusInnovations/keyhotee/issues/107
  */
  msgToOverwrite = nullptr;

  Sink.OnMessageSaved(*savedMsg, msgToOverwrite);

  if(msgToOverwrite != nullptr)
    Drafts->remove(*msgToOverwrite);
  }

void TMailProcessor::PrepareStorableMessage(const TIdentity& senderId,
  const TPhysicalMailMessage& sourceMsg, TStorableMessage* storableMsg)
  {
  /** It looks for me like another bug in backend. Even decrypted message object can be constructed
      directly by using interface of this class it is not sufficient to successfully store it in
      message_db. All operations performed while sending/receiving email must be performed (except
      transmission of course). So it must be first encrypted and next decrypted to properly fill
      actual decrypted message.
  */
  bts::bitchat::decrypted_message msg(sourceMsg);

  auto senderPrivKey = Profile->get_keychain().get_identity_key(senderId.dac_id_string);
  msg.sign(senderPrivKey);
  auto encMsg = msg.encrypt(senderId.public_key);
  encMsg.timestamp = fc::time_point::min();//now();

  encMsg.decrypt(senderPrivKey, *storableMsg);
  }

