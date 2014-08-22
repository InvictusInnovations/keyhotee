#include "ConnectionProcessor.hpp"

#include "ch/GuiUpdateSink.hpp"
#include "Mail/MailboxModel.hpp"

#include <bts/application.hpp>
#include <bts/bitchat/bitchat_private_message.hpp>

#include <fc/log/logger.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/thread/thread.hpp>

#include <QObject>

#include <atomic>
#include <mutex>

namespace
{
/// Helper class to be base for all notification implementations holding theirs specific data.
class ANotification
  {
  public:
    /// Sends a notification to the actual sink and destroys current object.
    virtual void Notify() = 0;

  protected:
    ANotification(IGuiUpdateSink& sink) : Sink(sink) {}
    virtual ~ANotification() {}

  /// Class attributes:
  protected:
    IGuiUpdateSink& Sink;
  };

/// The object receiving a signal sent by TThreadSafeGuiNotifier
class TReceiver : public QObject
  {
  Q_OBJECT
  public:
    virtual ~TReceiver() {}

  public slots:
    void notificationReceived(ANotification* notification)
      {
      notification->Notify();
      }
  };

} /// namespace anonymous

///////////////////////////////////////////////////////////////////////////////////////////////////
///                     TConnectionProcessor::TThreadSafeGuiNotifier                            ///
///////////////////////////////////////////////////////////////////////////////////////////////////

/** Helper GUI update sink implementation sending notifications to the actual sink object after
    switching to the GUI thread.
    To do it utilizes QT signal/slot mechanism.
*/
class TConnectionProcessor::TThreadSafeGuiNotifier : public QObject,
                                                     public IGuiUpdateSink
  {
  Q_OBJECT

  public:
    explicit TThreadSafeGuiNotifier(IGuiUpdateSink& actualUpdateSink) :
      Sink(actualUpdateSink)
      {
      /// Leave it as autoconnction to decide by QT engine how signal transmission should be done
      connect(this, SIGNAL(notificationSent(ANotification*)), &Receiver,
        SLOT(notificationReceived(ANotification*)), Qt::ConnectionType::QueuedConnection);
      }

    virtual ~TThreadSafeGuiNotifier() {}

  /// IGuiUpdateSink interface implementation:
    /// \see IGuiUpdateSink interface description.
    virtual void OnReceivedChatMessage(const TContact& sender, const TChatMessage& msg,
      const TTime& timeSent) override;
    /// \see IGuiUpdateSink interface description.
    virtual void OnReceivedAuthorizationMessage(const TAuthorizationMessage& msg,
      const TStoredMailMessage& header) override;
    /// \see IGuiUpdateSink interface description.
    virtual void OnReceivedMailMessage(const TStoredMailMessage& msg, const bool spam) override;
    /// \see IGuiUpdateSink interface description.
    virtual void OnReceivedUnsupportedMessage(const TDecryptedMessage& msg) override;
    /// \see IGuiUpdateSink interface description.
    virtual void OnMessageSaving() override;
    /// \see IGuiUpdateSink interface description.
    virtual void OnMessageSaved(const TStoredMailMessage& msg,
      const TStoredMailMessage* overwrittenOne) override;
    /// \see IGuiUpdateSink interface description.
    virtual void OnMessageGroupPending(unsigned int count) override;
    /// \see IGuiUpdateSink interface description.
    virtual void OnMessagePending(const TStoredMailMessage& msg,
      const TStoredMailMessage* savedDraftMsg) override;
    /// \see IGuiUpdateSink interface description.
    virtual void OnMessageGroupPendingEnd() override;
    /// \see IGuiUpdateSink interface description.
    virtual void OnMessageSendingStart() override;
    /// \see IGuiUpdateSink interface description.
    virtual void OnMessageSent(const TStoredMailMessage& pendingMsg,
      const TStoredMailMessage& sentMsg, const TDigest& digest) override;
    /// \see IGuiUpdateSink interface description.
    virtual void OnMessageSendingEnd() override;
    /// \see IGuiUpdateSink interface description.
    virtual void OnMissingSenderIdentity(const TRecipientPublicKey& senderId,
      const TPhysicalMailMessage& msg) override;

  private:
    class TReceivedChatMsg : public ANotification
      {
      public:
        static ANotification* Create(const TContact& sender, const TChatMessage& msg, const TTime& timeSent,
          IGuiUpdateSink& sink)
          {
          return new TReceivedChatMsg(sender, msg, timeSent, sink);
          }

      /// ANotification class reimplementation:
        virtual void Notify()
          {
          Sink.OnReceivedChatMessage(Sender, Msg, TimeSent);
          delete this;
          }

      private:
        TReceivedChatMsg(const TContact& sender, const TChatMessage& msg, const TTime& timeSent,
          IGuiUpdateSink& sink) : ANotification(sink), Sender(sender), Msg(msg), TimeSent(timeSent) {}
        virtual ~TReceivedChatMsg() {}

      private:
        TContact     Sender;
        TChatMessage Msg;
        TTime        TimeSent;
      };

    class TReceivedAuthorizationMsg : public ANotification
      {
      public:
        static ANotification* Create(const TAuthorizationMessage& msg, const TStoredMailMessage& header,
          IGuiUpdateSink& sink)
          {
          return new TReceivedAuthorizationMsg(msg, header, sink);
          }

      /// ANotification class reimplementation:
        virtual void Notify()
          {
          Sink.OnReceivedAuthorizationMessage(Msg, Header);
          delete this;
          }

      private:
        TReceivedAuthorizationMsg(const TAuthorizationMessage& msg, const TStoredMailMessage& header,
          IGuiUpdateSink& sink) : ANotification(sink), Msg(msg), Header(header) {}
        virtual ~TReceivedAuthorizationMsg() {}

      private:
        TAuthorizationMessage Msg;
        TStoredMailMessage Header;
      };

    class TReceivedMailMsg : public ANotification
      {
      public:
        static ANotification* Create(const TStoredMailMessage& msg, const bool spam, IGuiUpdateSink& sink)
          {
          return new TReceivedMailMsg(msg, spam, sink);
          }

      /// ANotification class reimplementation:
        virtual void Notify()
          {
          Sink.OnReceivedMailMessage(Msg, Spam);
          delete this;
          }

      private:
        TReceivedMailMsg(const TStoredMailMessage& msg, const bool spam, IGuiUpdateSink& sink) :
          ANotification(sink), Msg(msg), Spam(spam) {}
        virtual ~TReceivedMailMsg() {}

      private:
        TStoredMailMessage  Msg;
        bool                Spam;
      };

    class TReceivedUnsupportedMsg : public ANotification
      {
      public:
        static ANotification* Create(const TDecryptedMessage& msg, IGuiUpdateSink& sink)
          {
          return new TReceivedUnsupportedMsg(msg, sink);
          }

      /// ANotification class reimplementation:
        virtual void Notify()
          {
          Sink.OnReceivedUnsupportedMessage(Msg);
          delete this;
          }

      private:
        TReceivedUnsupportedMsg(const TDecryptedMessage& msg, IGuiUpdateSink& sink) :
          ANotification(sink), Msg(msg) {}
        virtual ~TReceivedUnsupportedMsg() {}

      private:
        TDecryptedMessage Msg;
      };

    class TNoDataNotification : public ANotification
      {
      public:
        typedef std::function<void()> TOperation;

        static ANotification* Create(const TOperation& op, IGuiUpdateSink& sink)
          {
          return new TNoDataNotification(op, sink);
          }

      /// ANotification class reimplementation:
        virtual void Notify()
          {
          Op();
          delete this;
          }

      private:
        TNoDataNotification(const TOperation& op, IGuiUpdateSink& sink) :
          ANotification(sink), Op(op) {}
        virtual ~TNoDataNotification() {}

      private:
        TOperation Op;
      };

    class TPendingOrSavedMailMessage : public ANotification
      {
      public:
        static ANotification* Create(const TStoredMailMessage& msg,
          const TStoredMailMessage* overwrittenOne, bool saved, IGuiUpdateSink& sink)
          {
          return new TPendingOrSavedMailMessage(msg, overwrittenOne, saved, sink);
          }

      /// ANotification class reimplementation:
        virtual void Notify()
          {
          if(Saved)
            Sink.OnMessageSaved(Msg, OverwrittenOnePtr);
          else
            Sink.OnMessagePending(Msg, OverwrittenOnePtr);

          delete this;
          }

      private:
        TPendingOrSavedMailMessage(const TStoredMailMessage& msg,
          const TStoredMailMessage* overwrittenOne, bool saved, IGuiUpdateSink& sink) :
          ANotification(sink),
          Msg(msg),
          OverwrittenOnePtr(nullptr),
          Saved(saved)
          {
          if(overwrittenOne != nullptr)
            {
            OverwrittenOne = *overwrittenOne;
            OverwrittenOnePtr = &OverwrittenOne;
            }
          }

        virtual ~TPendingOrSavedMailMessage() {}

      /// Class attributes:
      private:
        TStoredMailMessage        Msg;
        TStoredMailMessage        OverwrittenOne;
        const TStoredMailMessage* OverwrittenOnePtr;
        bool                      Saved;
      };

    class TPendingMessageGroup : public ANotification
      {
      public:
        static ANotification* Create(unsigned int count, IGuiUpdateSink& sink)
          {
          return new TPendingMessageGroup(count, sink);
          }

      /// ANotification class reimplementation:
        virtual void Notify()
          {
          Sink.OnMessageGroupPending(Count);
          delete this;
          }

      private:
        TPendingMessageGroup(unsigned int count, IGuiUpdateSink& sink) : ANotification(sink),
          Count(count) {}
        virtual ~TPendingMessageGroup() {}

      /// Class attributes:
      private:
        unsigned int Count;
      };

    class TSentMailMessage : public ANotification
      {
      public:
        static ANotification* Create(const TStoredMailMessage& pendingMsg,
          const TStoredMailMessage& sentMsg, const TDigest& digest, IGuiUpdateSink& sink)
          {
          return new TSentMailMessage(pendingMsg, sentMsg, digest, sink);
          }

      /// ANotification class reimplementation:
        virtual void Notify()
          {
          Sink.OnMessageSent(PendingMsg, SentMsg, Digest);
          delete this;
          }

      private:
        TSentMailMessage(const TStoredMailMessage& pendingMsg, const TStoredMailMessage& sentMsg,
          const TDigest& digest, IGuiUpdateSink& sink) : ANotification(sink),
          PendingMsg(pendingMsg),
          SentMsg(sentMsg),
          Digest(digest) {}

        virtual ~TSentMailMessage() {}

      /// Class attributes:
      private:
        TStoredMailMessage  PendingMsg;
        TStoredMailMessage  SentMsg;
        TDigest             Digest;
      };

    class TMissingSenderIdentity : public ANotification
      {
      public:
        static ANotification* Create(const TRecipientPublicKey& senderId,
          const TPhysicalMailMessage& msg, IGuiUpdateSink& sink)
          {
          return new TMissingSenderIdentity(senderId, msg, sink);
          }

      /// ANotification class reimplementation:
        virtual void Notify()
          {
          Sink.OnMissingSenderIdentity(SenderId, Msg);
          delete this;
          }

      private:
        TMissingSenderIdentity(const TRecipientPublicKey& senderId, const TPhysicalMailMessage& msg,
          IGuiUpdateSink& sink) : ANotification(sink),
          SenderId(senderId),
          Msg(msg) {}

        virtual ~TMissingSenderIdentity() {}

      /// Class attributes:
      private:
      TRecipientPublicKey  SenderId;
      TPhysicalMailMessage Msg;
      };

    Q_SIGNALS:
      /// Emmitted when some GUI notification should be propagated across threads.
      void notificationSent(ANotification* notification);

  private:
    IGuiUpdateSink& Sink;
    TReceiver       Receiver;
  };

void TConnectionProcessor::TThreadSafeGuiNotifier::OnReceivedChatMessage(const TContact& sender,
  const TChatMessage& msg, const TTime& timeSent)
  {
  ANotification* n = TReceivedChatMsg::Create(sender, msg, timeSent, Sink);
  emit notificationSent(n);
  }

void TConnectionProcessor::TThreadSafeGuiNotifier::OnReceivedAuthorizationMessage(
  const TAuthorizationMessage& msg, const TStoredMailMessage& header)
  {
  ANotification* n = TReceivedAuthorizationMsg::Create(msg, header, Sink);
  emit notificationSent(n);
  }

void TConnectionProcessor::TThreadSafeGuiNotifier::OnReceivedMailMessage(const TStoredMailMessage& msg, const bool spam)
  {
  ANotification* n = TReceivedMailMsg::Create(msg, spam, Sink);
  emit notificationSent(n);
  }

void TConnectionProcessor::TThreadSafeGuiNotifier::OnReceivedUnsupportedMessage(const bts::bitchat::decrypted_message& msg)
  {
    ANotification* n = TReceivedUnsupportedMsg::Create(msg, Sink);
    emit notificationSent(n);
  }

void TConnectionProcessor::TThreadSafeGuiNotifier::OnMessageSaving()
  {
  ANotification* n = TNoDataNotification::Create([=]() {Sink.OnMessageSaving();}, Sink);
  emit notificationSent(n);
  }

void TConnectionProcessor::TThreadSafeGuiNotifier::OnMessageSaved(const TStoredMailMessage& msg,
  const TStoredMailMessage* overwrittenOne)
  {
  ANotification* n = TPendingOrSavedMailMessage::Create(msg, overwrittenOne, true, Sink);
  emit notificationSent(n);
  }

void TConnectionProcessor::TThreadSafeGuiNotifier::OnMessageGroupPending(unsigned int count)
  {
  ANotification* n = TPendingMessageGroup::Create(count, Sink);
  emit notificationSent(n);
  }

void TConnectionProcessor::TThreadSafeGuiNotifier::OnMessagePending(const TStoredMailMessage& msg,
  const TStoredMailMessage* savedDraftMsg)
  {
  ANotification* n = TPendingOrSavedMailMessage::Create(msg, savedDraftMsg, false, Sink);
  emit notificationSent(n);
  }

void TConnectionProcessor::TThreadSafeGuiNotifier::OnMessageGroupPendingEnd()
  {
  ANotification* n = TNoDataNotification::Create([=]() {Sink.OnMessageGroupPendingEnd();}, Sink);
  emit notificationSent(n);
  }

void TConnectionProcessor::TThreadSafeGuiNotifier::OnMessageSendingStart()
  {
  ANotification* n = TNoDataNotification::Create([=]() {Sink.OnMessageSendingStart();}, Sink);
  emit notificationSent(n);
  }

void TConnectionProcessor::TThreadSafeGuiNotifier::OnMessageSent(const TStoredMailMessage& pendingMsg,
  const TStoredMailMessage& sentMsg, const TDigest& digest)
  {
  ANotification* n = TSentMailMessage::Create(pendingMsg, sentMsg, digest, Sink);
  emit notificationSent(n);
  }

void TConnectionProcessor::TThreadSafeGuiNotifier::OnMessageSendingEnd()
  {
  ANotification* n = TNoDataNotification::Create([=]() {Sink.OnMessageSendingEnd();}, Sink);
  emit notificationSent(n);
  }

void TConnectionProcessor::TThreadSafeGuiNotifier::OnMissingSenderIdentity(
  const TRecipientPublicKey& senderId, const TPhysicalMailMessage& msg)
  {
  ANotification* n = TMissingSenderIdentity::Create(senderId, msg, Sink);
  emit notificationSent(n);
  }

///////////////////////////////////////////////////////////////////////////////////////////////////
///                             TConnectionProcessor::TOutboxQueue                              ///
///////////////////////////////////////////////////////////////////////////////////////////////////

class TConnectionProcessor::TOutboxQueue
  {
  public:
    TOutboxQueue(TConnectionProcessor& processor, const bts::profile_ptr& profile) :
      Processor(processor)
      {
      Profile = profile;
      App = bts::application::instance();

      Outbox = profile->get_pending_db();
      Sent = profile->get_sent_db();
      CancelPromise = new fc::promise<void>;

      checkForAvailableConnection();
      }

    /** Allows to add new pending message to the sending queue.
        \param senderId      - identity chosen to be specified as mail sender,
        \param msg           - mail message to be sent,
        \param msg_type      - message type: Normal, Forward, Reply
        \param savedDraftMsg - optional, can be nullptr. If not null, it means that previously saved
                               draft message is about to send (it should be removed from Draft
                               folder).
    */
    void AddPendingMessage(const TIdentity& senderId, const TPhysicalMailMessage& msg,
      const TMsgType msg_type, const TStoredMailMessage* savedDraftMsg);

    /** Allows to add new pending message to the sending queue.
    \param senderId      - identity chosen to be specified as maessage sender,
    \param msg           - authorization message to be sent,
    */
    void AddPendingAuthoMsg(const TIdentity& senderId, const TRequestMessage& msg);

    bool AnyOperationsPending() const;

    /// Returns length of the queue.
    unsigned int GetLength() const;

    bool isTransmissionLoopActive() const
      {
      return TransferLoopComplete.valid() && TransferLoopComplete.ready() == false;
      }

    /// Method dedicated to cancel any transmission ie just before quiting the app.
    void StopTransmission()
      {
      bool transferActive = isTransmissionLoopActive();
      bool connectionActive = ConnectionCheckComplete.valid() &&
        ConnectionCheckComplete.ready() == false;
      
      if(transferActive || connectionActive)
        {
        CancelPromise->set_value();

        if(ConnectionCheckComplete.valid())
          {
          ConnectionCheckComplete.cancel_and_wait();
          }

        if(TransferLoopComplete.valid())
          {
          TransferLoopComplete.cancel_and_wait();
          }
        }
      }

    void Release()
      {
      StopTransmission();

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

    bool isMailConnected() const
      {
      return App->is_mail_connected();
      }

    void connectionCheckingLoop();
    void startTransmission()
      {
      if(!TransferLoopComplete.valid() || TransferLoopComplete.ready())
        TransferLoopComplete = fc::async([=]{ transmissionLoop(); });
      }

    void checkForAvailableConnection()
      {
      if(ConnectionCheckComplete.valid() == false || ConnectionCheckComplete.ready())
        ConnectionCheckComplete = fc::async([=]{ connectionCheckingLoop(); });
      }

    bool isCancelled() const
      {
      return CancelPromise->ready();
      }

    bool fetchNextMessage(TStoredMailMessage* storedMsg, TPhysicalMailMessage* mail_msg,
      TRequestMessage* auth_msg, bool* auth_flag);
    bool transferMessage(const TRecipientPublicKey& senderId, const TPhysicalMailMessage& msg);
    bool transferAuthMsg(const TRecipientPublicKey& senderId, const TRequestMessage& auth_msg);
    void sendMail(const TPhysicalMailMessage& email, const TRecipientPublicKey& to,
      const fc::ecc::private_key& from);
    void sendAuthMsg(const TRequestMessage& auth_msg, const TRecipientPublicKey& to,
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
    TConnectionProcessor&        Processor;
    bts::profile_ptr       Profile;
    bts::application_ptr   App;
    TMessageDB             Outbox;
    TMessageDB             Sent;
    fc::future<void>       TransferLoopComplete;
    fc::future<void>       ConnectionCheckComplete;
    fc::promise<void>::ptr CancelPromise;
    mutable std::mutex     OutboxDbLock;
  };

void TConnectionProcessor::TOutboxQueue::AddPendingMessage(const TIdentity& senderId,
  const TPhysicalMailMessage& msg, const TMsgType msg_type, const TStoredMailMessage* savedDraftMsg)
  {
  std::lock_guard<std::mutex> guard(OutboxDbLock);

  TStorableMessage storableMsg;
  Processor.PrepareStorableMessage(senderId, msg, &storableMsg);
  TStoredMailMessage storedMsg = Outbox->store_message(storableMsg, nullptr);
  switch (msg_type)
  {
    case TMsgType::Reply:
      storedMsg.setTempReply();
      break;
    case TMsgType::Forward:
      storedMsg.setTempForwa();
      break;
    default:
      storedMsg.clearTemp();
      break;
  }
  Outbox->store_message_header(storedMsg);
  Processor.Sink->OnMessagePending(storedMsg, savedDraftMsg);

  /// Try to start thread checking connection and next potential transmission
  checkForAvailableConnection();
  }

void TConnectionProcessor::TOutboxQueue::AddPendingAuthoMsg(const TIdentity& senderId, const TRequestMessage& msg)
{
  std::lock_guard<std::mutex> guard(OutboxDbLock);

  TStorableMessage storableMsg;
  Processor.PrepareStorableAuthMsg(senderId, msg, &storableMsg);
  TStoredMailMessage storedMsg = Outbox->store_message(storableMsg, nullptr);

  /// Try to start thread checking connection and next potential transmission
  checkForAvailableConnection();
}

bool TConnectionProcessor::TOutboxQueue::AnyOperationsPending() const
  {
  bool transferLoopActive = isTransmissionLoopActive();
  std::lock_guard<std::mutex> guard(OutboxDbLock);
  return transferLoopActive ? Outbox->fetch_headers(TPhysicalMailMessage::type).empty() : false;
  }

unsigned int TConnectionProcessor::TOutboxQueue::GetLength() const
  {
  bool transferLoopCompleted = TransferLoopComplete.valid() == false || TransferLoopComplete.ready();
  std::lock_guard<std::mutex> guard(OutboxDbLock);
  return transferLoopCompleted ? 0 : Outbox->fetch_headers(TPhysicalMailMessage::type).size();
  }

void TConnectionProcessor::TOutboxQueue::transmissionLoop()
  {
  bool notificationSent = false;

  TPhysicalMailMessage  mail_msg;
  TRequestMessage       auth_msg;
  TStoredMailMessage    storedMsg;
  bool                  auth_flag = false;

  while(!isCancelled() && fetchNextMessage(&storedMsg, &mail_msg, &auth_msg, &auth_flag))
    {
    if(!notificationSent)
      {
      Processor.Sink->OnMessageSendingStart();
      notificationSent = true;
      }

    if(auth_flag)
    {
      if(transferAuthMsg(storedMsg.from_key, auth_msg))
        Outbox->remove_message(storedMsg);
    }
    else
    {
      if(transferMessage(storedMsg.from_key, mail_msg))
        moveMsgToSentDB(storedMsg, mail_msg);
    }

    if(isCancelled())
      break;

    fc::usleep(fc::milliseconds(250));
    }

  if(notificationSent)
    Processor.Sink->OnMessageSendingEnd();
  }

void TConnectionProcessor::TOutboxQueue::connectionCheckingLoop()
  {
  do
    {
    if(isMailConnected())
      {
      startTransmission();
      return;
      }

    fc::usleep(fc::milliseconds(250));
    }
  while(CancelPromise->ready() == false);
  }

bool TConnectionProcessor::TOutboxQueue::fetchNextMessage(TStoredMailMessage* storedMsg,
  TPhysicalMailMessage* mail_msg, TRequestMessage* auth_msg, bool* auth_flag)
  {
  assert(storedMsg != nullptr);
  assert(mail_msg != nullptr);
  assert(auth_msg != nullptr);

  std::lock_guard<std::mutex> guard(OutboxDbLock);

  try
    {
    /// FIXME - message_db interface is terrible - there should be a way to query just for 1 object
    auto pendingMsgHeaders = Outbox->fetch_headers(TPhysicalMailMessage::type);
    auto pendingAuthHeaders = Outbox->fetch_headers(TRequestMessage::type);
    if(pendingMsgHeaders.empty() && pendingAuthHeaders.empty())
      return false;

    if(!pendingMsgHeaders.empty())
    {
      *storedMsg = pendingMsgHeaders.front();
      auto rawData = Outbox->fetch_data(storedMsg->digest);
      *mail_msg = fc::raw::unpack<TPhysicalMailMessage>(rawData);
      *auth_flag = false;
    }
    else if(!pendingAuthHeaders.empty())
    {
      *storedMsg = pendingAuthHeaders.front();
      auto rawData = Outbox->fetch_data(storedMsg->digest);
      *auth_msg = fc::raw::unpack<TRequestMessage>(rawData);
      *auth_flag = true;
    }

    return true;
    }
  catch(const fc::exception& e)
    {
    elog("${e}", ("e", e.to_detail_string()));
    return false;
    }
  }

bool TConnectionProcessor::TOutboxQueue::transferMessage(const TRecipientPublicKey& senderId,
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
        {
        if(isCancelled())
          return false;
        sendMail(msgToSend, public_key, senderPrivKey);
        }

      for(const auto& public_key : msgToSend.cc_list)
        {
        if(isCancelled())
          return false;
        sendMail(msgToSend, public_key, senderPrivKey);
        }

      for(const auto& public_key : bccList)
        {
        if(isCancelled())
          return false;
        sendMail(msgToSend, public_key, senderPrivKey);
        }

      sendStatus = true;
      }
    else
      {
      Processor.Sink->OnMissingSenderIdentity(senderId, msg);
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

bool TConnectionProcessor::TOutboxQueue::transferAuthMsg(const TRecipientPublicKey& senderId,
  const TRequestMessage& auth_msg)
{
  bool sendStatus = false;

  try
  {
    bts::extended_private_key senderPrivKey;
    if(findIdentityPrivateKey(senderId, &senderPrivKey))
      sendAuthMsg(auth_msg, auth_msg.recipient, senderPrivKey);

    sendStatus = true;
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
void TConnectionProcessor::TOutboxQueue::sendMail(const TPhysicalMailMessage& email,
  const TRecipientPublicKey& to, const fc::ecc::private_key& from)
  {
  if(isMailConnected())
    {
    App->send_email(email, to, from);
    return;
    }

  FC_THROW("No connection to execute send_email"); 
  }

inline
void TConnectionProcessor::TOutboxQueue::sendAuthMsg(const TRequestMessage& auth_msg,
  const TRecipientPublicKey& to, const fc::ecc::private_key& from)
{
  if(isMailConnected())
  {
    App->send_contact_request(auth_msg, to, from);
    return;
  }

  FC_THROW("No connection to execute send_contact_request");
}

inline
bool TConnectionProcessor::TOutboxQueue::findIdentity(const TRecipientPublicKey& senderId,
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
bool TConnectionProcessor::TOutboxQueue::findIdentityPrivateKey(const TRecipientPublicKey& senderId,
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

void TConnectionProcessor::TOutboxQueue::moveMsgToSentDB(const TStoredMailMessage& pendingMsg,
  const TPhysicalMailMessage& sentMsg)
  {
  try
    {
    TIdentity id;
    bool result = findIdentity(pendingMsg.from_key, &id);
    assert(result);

    TStorableMessage storableMsg;
    Processor.PrepareStorableMessage(id, sentMsg, &storableMsg);

    TStoredMailMessage savedMsg = Sent->store_message(storableMsg, nullptr);
    savedMsg.setRead();
    Sent->store_message_header(savedMsg);

    Processor.Sink->OnMessageSent(pendingMsg, savedMsg, sentMsg.src_msg_id);

    std::lock_guard<std::mutex> guard(OutboxDbLock);

    Outbox->remove_message(pendingMsg);
    }
  catch(const fc::exception& e)
    {
    elog("${e}", ("e", e.to_detail_string()));
    }
  }

///////////////////////////////////////////////////////////////////////////////////////////////////
///                                    TConnectionProcessor                                     ///
///////////////////////////////////////////////////////////////////////////////////////////////////

TConnectionProcessor::TConnectionProcessor(IGuiUpdateSink& updateSink,
  const bts::profile_ptr& loadedProfile) :
  Profile(loadedProfile),
  TransmissionCancelled(false),
  ReceivingMail(false)
  {
  Sink = new TThreadSafeGuiNotifier(updateSink);

  App = bts::application::instance();
  
  App->set_application_delegate(this);

  Drafts = Profile->get_draft_db();
  OutboxQueue = new TOutboxQueue(*this, Profile);

  updateOptions();
  }

TConnectionProcessor::~TConnectionProcessor()
  {
  App->set_application_delegate(nullptr);
  delete Sink;
  Sink = nullptr;
  OutboxQueue->Release();
  OutboxQueue = nullptr;
  }

void TConnectionProcessor::Send(const TIdentity& senderId, const TPhysicalMailMessage& msg,
  const TMsgType msg_type, const TStoredMailMessage* savedDraftMsg)
  {
  OutboxQueue->AddPendingMessage(senderId, msg, msg_type, savedDraftMsg);
  }

IMailProcessor::TStoredMailMessage 
TConnectionProcessor::Save(const TIdentity& senderId, const TPhysicalMailMessage& sourceMsg,
  const TMsgType msg_type, const TStoredMailMessage* msgBeingReplaced)
  {
  Sink->OnMessageSaving();

  TStorableMessage storableMsg;
  PrepareStorableMessage(senderId, sourceMsg, &storableMsg);
  //Modify digest by updating signature time for the draft email.
  //Note that signature time is not true signature time of send, but
  //time when this version of draft email is being saved.
  storableMsg.sig_time = fc::time_point::now();
  TStoredMailMessage savedMsg = Drafts->store_message(storableMsg,msgBeingReplaced);
  switch (msg_type)
  {
    case TMsgType::Reply:
      savedMsg.setTempReply();
      break;
    case TMsgType::Forward:
      savedMsg.setTempForwa();
      break;
    default:
      savedMsg.clearTemp();
      break;
  }
  Drafts->store_message_header(savedMsg);
  Sink->OnMessageSaved(savedMsg, msgBeingReplaced);
  return savedMsg;
  }

unsigned int TConnectionProcessor::GetPeerConnectionCount() const
  {
  return static_cast<unsigned int>(App->get_network()->get_connections().size());
  }

bool TConnectionProcessor::IsMailConnected() const
  {
  return App->is_mail_connected();
  }

void TConnectionProcessor::SendAuth(const TCurrIdentity& senderId, const TRequestMessage& msg)
{
  OutboxQueue->AddPendingAuthoMsg(senderId, msg);
}

void TConnectionProcessor::storeAuthorization(const TCurrIdentity& senderId, const TRequestMessage& src_msg,
                                              const TStoredMessage& msg_header)
{
  try
  {
    auto request_db = Profile->get_request_db();
    request_db->remove_message(msg_header);

    auto privKey = Profile->get_keychain().get_identity_key(senderId.dac_id_string);
    bts::bitchat::decrypted_message msg(src_msg);
    msg.sign(privKey);
    auto encMsg = msg.encrypt(senderId.public_key);
    encMsg.timestamp = fc::time_point::now();
    encMsg.decrypt(privKey, msg);
    Profile->get_auth_db()->store_message(msg, nullptr);
  }
  catch(const fc::exception& e)
  {
    elog("${e}", ("e", e.to_detail_string()));
  }
}

bool TConnectionProcessor::CanQuit(bool* canBreak /*= nullptr*/) const
  {
  if(canBreak != nullptr)
    *canBreak = !ReceivingMail; /// Incoming mail transmission is not breakable

  return !OutboxQueue->isTransmissionLoopActive() && 
         !ReceivingMail &&
         !OutboxQueue->AnyOperationsPending();
  }

void TConnectionProcessor::CancelTransmission()
  {
  assert(ReceivingMail == false);

  OutboxQueue->StopTransmission();
  }

void TConnectionProcessor::connection_count_changed(unsigned int count)
  {
  /// Looks like this notification is not yet sent - then do nothing right now even could be useful
  }

bool TConnectionProcessor::receiving_mail_message()
  {
  ReceivingMail = true;
  return !TransmissionCancelled;
  }

void TConnectionProcessor::received_text(const bts::bitchat::decrypted_message& msg)
{
  try
  {
    auto aBook = Profile->get_addressbook();
    fc::optional<bts::addressbook::wallet_contact> optContact;

    if(msg.from_key)
      optContact = aBook->get_contact_by_public_key(*msg.from_key);

    if(optContact)
    {
      wlog("Received text from known contact!");
      if(!_chat_allow_flag ||
        (*optContact).auth_status == bts::addressbook::authorization_status::accepted ||
        (*optContact).auth_status == bts::addressbook::authorization_status::accepted_chat)
      {
        Profile->get_chat_db()->store_message(msg, nullptr);
        auto chatMsg = msg.as<bts::bitchat::private_text_message>();
        Sink->OnReceivedChatMessage(*optContact, chatMsg, msg.sig_time);
      }
      else
      {
        wlog("Chat message from known contact rejected!");
      }
    }
    else
    {
      elog("Received chat message from unknown contact/missing sender - ignoring");
    }
  }
  catch(const fc::exception& e)
  {
    elog("${e}", ("e", e.to_detail_string()));
  }
}

void TConnectionProcessor::received_email(const bts::bitchat::decrypted_message& msg)
{
  try
  {
    bool allow = false;
    if(_mail_allow_flag)
    {
      if(isMyIdentity(*msg.from_key))
        allow = true;
      else
      {
        auto aBook = Profile->get_addressbook();
        fc::optional<bts::addressbook::wallet_contact> optContact;

        if(msg.from_key)
          optContact = aBook->get_contact_by_public_key(*msg.from_key);

        if(optContact)
          if((*optContact).auth_status == bts::addressbook::authorization_status::accepted ||
             (*optContact).auth_status == bts::addressbook::authorization_status::accepted_mail)
            allow = true;
      }
    }
    else
      allow = true;

    if(allow)
    {
      auto header = Profile->get_inbox_db()->store_message(msg, nullptr);
      wlog("email stored in database");
      Sink->OnReceivedMailMessage(header, false);
      wlog("gui notified");
      ReceivingMail = false;
    }
    else
    {
      if(_save_spam_flag)
      {
        wlog("email message moved to spam!");
        auto header = Profile->get_spam_db()->store_message(msg, nullptr);
        Sink->OnReceivedMailMessage(header, true);
        ReceivingMail = false;
      }
      else
        wlog("*** email message rejected!");
    }
  }
  catch(const fc::exception& e)
  {
    elog("${e}", ("e", e.to_detail_string()));
  }
}

void TConnectionProcessor::received_request(const bts::bitchat::decrypted_message& msg)
{
  try
  {
    if(msg.from_key)
    {
      auto aBook = Profile->get_addressbook();
      fc::optional<bts::addressbook::wallet_contact> optContact;
      optContact = aBook->get_contact_by_public_key(*msg.from_key);

      if(!optContact ||
        (optContact && (*optContact).auth_status != bts::addressbook::authorization_status::i_block))
      {
        auto header = Profile->get_request_db()->store_message(msg, nullptr);
        auto req_msg = msg.as<bts::bitchat::private_contact_request_message>();
        Sink->OnReceivedAuthorizationMessage(req_msg, header);
      }
      else
        ilog("Received auth. message from blocked contact - ignoring");
    }
    else
    {
      elog("Received auth. message with missing sender key - ignoring");
    }
  }
  catch(const fc::exception& e)
  {
    elog("${e}", ("e", e.to_detail_string()));
  }
}

void TConnectionProcessor::received_unsupported_msg(const bts::bitchat::decrypted_message& msg)
{
  Sink->OnReceivedUnsupportedMessage(msg);
}

void TConnectionProcessor::message_transmission_finished(bool success)
  {
  ReceivingMail = false;
  }

void TConnectionProcessor::updateOptions()
{
  QString profile_name = QString::fromStdWString(Profile->get_name());
  QString settings_file = "keyhotee_";
  settings_file.append(profile_name);
  QSettings settings("Invictus Innovations", settings_file);

  _chat_allow_flag = settings.value("AllowChat", "").toBool();
  _mail_allow_flag = settings.value("AllowMail", "").toBool();
  _save_spam_flag = settings.value("SaveSpam", "").toBool();
}

bool TConnectionProcessor::isMyIdentity(const TRecipientPublicKey& senderId)
{
  for(const TIdentity& id : Profile->identities())
  {
    if(id.public_key == senderId)
      return true;
  }

  return false;
}

void TConnectionProcessor::PrepareStorableMessage(const TIdentity& senderId,
  const TPhysicalMailMessage& sourceMsg, TStorableMessage* storableMsg)
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

void TConnectionProcessor::PrepareStorableAuthMsg(const TIdentity& senderId,
  const TRequestMessage& sourceMsg, TStorableMessage* storableMsg)
{
  bts::bitchat::decrypted_message msg(sourceMsg);

  auto senderPrivKey = Profile->get_keychain().get_identity_key(senderId.dac_id_string);
  msg.sign(senderPrivKey);
  auto encMsg = msg.encrypt(senderId.public_key);
  encMsg.timestamp = fc::time_point::now();

  encMsg.decrypt(senderPrivKey, *storableMsg);
}

#include "ConnectionProcessor.moc"

