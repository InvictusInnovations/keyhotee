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

TMailProcessor::TMailProcessor(KeyhoteeMainWindow* mainWindow,
  const bts::profile_ptr& loadedProfile) :
  MainWindow(mainWindow),
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
  auto my_priv_key = Profile->get_keychain().get_identity_key(senderId.dac_id_string);
  auto app = bts::application::instance();

  for(const auto& public_key : msg.to_list)
    app->send_email(msg, public_key, my_priv_key);

  for(const auto& public_key : msg.cc_list)
    app->send_email(msg, public_key, my_priv_key);

  for(const auto& public_key : bccList)
    app->send_email(msg, public_key, my_priv_key);
  }

void TMailProcessor::Save(const TIdentity& senderId, const TPhysicalMailMessage& msg,
  const TRecipientPublicKeys& bccList)
  {
  }

