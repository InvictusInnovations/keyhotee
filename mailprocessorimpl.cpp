#include "mailprocessorimpl.hpp"

#include <bts/application.hpp>

#include <fc/log/logger.hpp>
#include <fc/thread/thread.hpp>

#include <atomic>
#include <mutex>

class TMailProcessor::TOutboxQueue
  {
  public:
    TOutboxQueue(TMailProcessor& processor, const bts::profile_ptr& profile) :
      Processor(processor)
      {
      Profile = profile;
      App = bts::application::instance();

      Outbox = profile->get_pending_db();
      Sent = profile->get_sent_db();
      CancelPromise = new fc::promise<void>;

      //checkForAvailableConnection();
      }

    /** Allows to add new pending message to the sending queue.
        \param senderId      - identity chosen to be specified as mail sender,
        \param msg           - mail message to be sent,
        \param savedDraftMsg - optional, can be nullptr. If not null, it means that previously saved
                               draft message is about to send (it should be removed from Draft
                               folder).
    */
    void AddPendingMessage(const TIdentity& senderId, const TPhysicalMailMessage& msg,
      const TStoredMailMessage* savedDraftMsg);

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

    void transmissionLoop();
    bool isConnected() const
      {
      auto network = App->get_network();
      return static_cast<unsigned int>(network->get_connections().size()) > 0;
      }

    void connectionCheckingLoop();
    void startTransmission()
      {
      if(TransferLoopComplete.valid() == false || TransferLoopComplete.ready())
        TransferLoopComplete = fc::async([=]{ transmissionLoop(); });
      }

    void checkForAvailableConnection()
      {
      if(ConnectionCheckComplete.valid() == false || ConnectionCheckComplete.ready())
        ConnectionCheckComplete = fc::async([=]{ connectionCheckingLoop(); });
      }

    bool fetchNextMessage(TStoredMailMessage* storedMsg, TPhysicalMailMessage* storage);
    bool transferMessage(const TRecipientPublicKey& senderId, const TPhysicalMailMessage& msg);
    void sendMail(const TPhysicalMailMessage& email, const TRecipientPublicKey& to,
      const fc::ecc::private_key& from);

    /** Allows to get identity associated to given public key. Returns false if there is no
        associated identity to given public key.
    */
    bool findIdentity(const TRecipientPublicKey& senderId, TIdentity* identity) const;
    /** Allows to get private key associated to given public key (held by one of defined identities).
        Returns false if there is no associated identity to given public key.
    */
    bool findIdentityPrivateKey(const TRecipientPublicKey& senderId,
      bts::extended_private_key* key) const;
    /// Allows to move already sent message from Outbox DB into Sent DB.
    void moveMsgToSentDB(const TStoredMailMessage& storedMsg, const TPhysicalMailMessage& sentMsg);

  private:
    TMailProcessor&        Processor;
    bts::profile_ptr       Profile;
    bts::application_ptr   App;
    TMessageDB             Outbox;
    TMessageDB             Sent;
    fc::future<void>       TransferLoopComplete;
    fc::future<void>       ConnectionCheckComplete;
    fc::promise<void>::ptr CancelPromise;
    std::mutex             OutboxDbLock;
  };

void TMailProcessor::TOutboxQueue::AddPendingMessage(const TIdentity& senderId,
  const TPhysicalMailMessage& msg, const TStoredMailMessage* savedDraftMsg)
  {
  std::lock_guard<std::mutex> guard(OutboxDbLock);

  TStorableMessage storableMsg;
  Processor.PrepareStorableMessage(senderId, msg, &storableMsg);
  TStoredMailMessage storedMsg = Outbox->store_message(storableMsg, nullptr);
  Processor.Sink.OnMessagePending(storedMsg, savedDraftMsg);
  }

bool TMailProcessor::TOutboxQueue::AnyOperationsPending() const
  {
  bool transferLoopCompleted = TransferLoopComplete.valid() == false || TransferLoopComplete.ready();
  return transferLoopCompleted ? false : Outbox->fetch_headers(TPhysicalMailMessage::type).empty();
  }

unsigned int TMailProcessor::TOutboxQueue::GetLength() const
  {
  bool transferLoopCompleted = TransferLoopComplete.valid() == false || TransferLoopComplete.ready();
  return transferLoopCompleted ? 0 : Outbox->fetch_headers(TPhysicalMailMessage::type).size();
  }

void TMailProcessor::TOutboxQueue::transmissionLoop()
  {
  bool notificationSent = false;

  while(CancelPromise->ready() == false)
    {
    if(notificationSent == false)
      {
      Processor.Sink.OnMessageSendingStart();
      notificationSent = true;
      }

    TPhysicalMailMessage msg;
    TStoredMailMessage   storedMsg;
    if(fetchNextMessage(&storedMsg, &msg) && transferMessage(storedMsg.from_key, msg))
      moveMsgToSentDB(storedMsg, msg);

    fc::usleep(fc::milliseconds(250));
    }

  if(notificationSent)
    Processor.Sink.OnMessageSendingEnd();
  }

void TMailProcessor::TOutboxQueue::connectionCheckingLoop()
  {
  do
    {
    if(isConnected())
      {
      startTransmission();
      break;
      }

    fc::usleep(fc::milliseconds(250));
    }
  while(1);
  }

bool TMailProcessor::TOutboxQueue::fetchNextMessage(TStoredMailMessage* storedMsg,
  TPhysicalMailMessage* storage)
  {
  assert(storedMsg != nullptr);
  assert(storage != nullptr);

  std::lock_guard<std::mutex> guard(OutboxDbLock);

  /// FIXME - message_db interface is terrible - there should be a way to query just for 1 object
  auto pendingMsgHeaders = Outbox->fetch_headers(TPhysicalMailMessage::type);
  if(pendingMsgHeaders.empty())
    return false;

  *storedMsg = pendingMsgHeaders.front();
  auto rawData = Outbox->fetch_data(storedMsg->digest);
  *storage = fc::raw::unpack<TPhysicalMailMessage>(rawData);

  return true;
  }

bool TMailProcessor::TOutboxQueue::transferMessage(const TRecipientPublicKey& senderId,
  const TPhysicalMailMessage& msg)
  {
  bool sendStatus = false;

  try
    {
    bts::extended_private_key senderPrivKey;
    if(findIdentityPrivateKey(senderId, &senderPrivKey))
      {
      TPhysicalMailMessage msgToSend(msg);
      TRecipientPublicKeys bccList(msg.bcc_list);
      /// \warning Message to be sent must have cleared bcc list.
      msgToSend.bcc_list.clear();

      size_t totalRecipientCount = msgToSend.to_list.size() + msgToSend.cc_list.size() + bccList.size();

      for(const auto& public_key : msgToSend.to_list)
        sendMail(msgToSend, public_key, senderPrivKey);

      for(const auto& public_key : msgToSend.cc_list)
        sendMail(msgToSend, public_key, senderPrivKey);

      for(const auto& public_key : bccList)
        sendMail(msgToSend, public_key, senderPrivKey);

      sendStatus = true;
      }
    else
      {
      Processor.Sink.OnMissingSenderIdentity(senderId, msg);
      sendStatus = false;
      }
    }
  catch(const fc::exception& e)
    {
    sendStatus = false;
    elog("${e}", ("e", e.to_detail_string()));
    /// Probably connection related error, try to start it again
    checkForAvailableConnection();
    }

  return sendStatus;
  }

inline
void TMailProcessor::TOutboxQueue::sendMail(const TPhysicalMailMessage& email,
  const TRecipientPublicKey& to, const fc::ecc::private_key& from)
  {
  if(isConnected())
    {
    App->send_email(email, to, from);
    return;
    }

  FC_THROW("No connection to execute send_email"); 
  }

inline
bool TMailProcessor::TOutboxQueue::findIdentity(const TRecipientPublicKey& senderId,
  TIdentity* identity) const
  {
  *identity = TIdentity();

  for(const TIdentity& id : Profile->identities())
    {
    if(id.public_key == senderId)
      {
      *identity = id;
      return true;
      }
    }

  return false;
  }

inline
bool TMailProcessor::TOutboxQueue::findIdentityPrivateKey(const TRecipientPublicKey& senderId,
  bts::extended_private_key* key) const
  {
  *key = bts::extended_private_key();

  TIdentity id;
  if(findIdentity(senderId, &id))
    {
    *key = Profile->get_keychain().get_identity_key(id.dac_id_string);
    return true;
    }

  return false;
  }

void TMailProcessor::TOutboxQueue::moveMsgToSentDB(const TStoredMailMessage& pendingMsg,
  const TPhysicalMailMessage& sentMsg)
  {
  TIdentity id;
  bool result = findIdentity(pendingMsg.from_key, &id);
  assert(result);

  TStorableMessage storableMsg;
  Processor.PrepareStorableMessage(id, sentMsg, &storableMsg);

  TStoredMailMessage savedMsg = Sent->store_message(storableMsg, nullptr);
  Processor.Sink.OnMessageSent(pendingMsg, savedMsg);

  std::lock_guard<std::mutex> guard(OutboxDbLock);

  Outbox->remove_message(pendingMsg);
  }

TMailProcessor::TMailProcessor(IUpdateSink& updateSink,
  const bts::profile_ptr& loadedProfile) :
  Sink(updateSink),
  Profile(loadedProfile)
  {
  Drafts = Profile->get_draft_db();
  OutboxQueue = new TOutboxQueue(*this, Profile);
  }

TMailProcessor::~TMailProcessor()
  {
  OutboxQueue->Release();
  }

void TMailProcessor::Send(const TIdentity& senderId, const TPhysicalMailMessage& msg,
  const TStoredMailMessage* savedDraftMsg)
  {
  const bool outboxSupport = false;
  if(outboxSupport)
    {
    OutboxQueue->AddPendingMessage(senderId, msg, savedDraftMsg);
    }
  else
    {
    TStorableMessage storableMsg;
    PrepareStorableMessage(senderId, msg, &storableMsg);
    auto outbox = Profile->get_pending_db();
    auto sent = Profile->get_sent_db();
    TStoredMailMessage pendingMsg = outbox->store_message(storableMsg, nullptr);

    Sink.OnMessagePending(pendingMsg, savedDraftMsg);

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
    
    TStoredMailMessage sentMsg = sent->store_message(storableMsg, nullptr);
    Sink.OnMessageSent(pendingMsg, sentMsg);
    }
  }

IMailProcessor::TStoredMailMessage 
TMailProcessor::Save(const TIdentity& senderId, const TPhysicalMailMessage& sourceMsg,
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

