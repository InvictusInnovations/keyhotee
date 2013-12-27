#ifndef __MAILPROCESSORIMPL_HPP
#define __MAILPROCESSORIMPL_HPP

#include "ch/mailprocessor.hpp"

#include <bts/profile.hpp>

/** Implementation of mail processor storing sent mail in actual folders (outbox and next in sent db).
    Send operation is performed in separate thread.
    Save operation puts given mail message into drafts db.
*/
class TMailProcessor : public IMailProcessor
  {
  public:
    TMailProcessor(IUpdateSink& updateSink, const bts::profile_ptr& loadedProfile);
    virtual ~TMailProcessor();

  /// IMailProcessor interface implementation:
    /// \see IMailProcessor interface description.
    virtual void Send(const TIdentity& senderId, const TPhysicalMailMessage& msg,
      const TStoredMailMessage* savedDraftMsg) override;
    /// \see IMailProcessor interface description.
    virtual void Save(const TIdentity& senderId, const TPhysicalMailMessage& msg,
      const TStoredMailMessage* msgToOverwrite, TStoredMailMessage* savedMsg) override;

  /// Other implementation helpers:

    /** If any message sent is in progress asks user to stop it.
        Returns true if application exit can be continued, false otherwise.
    */
    bool CanQuit() const;

  private:
    typedef bts::bitchat::decrypted_message TStorableMessage;
    void PrepareStorableMessage(const TIdentity& senderId, const TPhysicalMailMessage& msg,
      TStorableMessage* storableMsg);

  /// Class attributes:
  private:
    class TOutboxQueue;
    typedef bts::bitchat::message_db_ptr TMessageDB;

    /// Sink to notify client about performed operations..
    IUpdateSink&      Sink;
    bts::profile_ptr  Profile;
    TMessageDB        Drafts;
    TOutboxQueue*     OutboxQueue;
  };

#endif /// __MAILPROCESSORIMPL_HPP

