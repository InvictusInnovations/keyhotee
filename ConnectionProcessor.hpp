#ifndef __CONNECTIONPROCESSOR_HPP
#define __CONNECTIONPROCESSOR_HPP

#include "ch/authprocessor.hpp"
#include "ch/connectionstatusds.h"
#include "ch/mailprocessor.hpp"

#include "Mail/MailboxModelRoot.hpp"

#include <bts/application.hpp>

#include <atomic>

class IGuiUpdateSink;

/** Implementation of mail processor storing sent mail in actual folders (outbox and next in sent db).
    Send operation is performed in separate thread.
    Save operation puts given mail message into drafts db.
    This class is also responsible for receiving backend notifications related to networking, message
    transfer and (safely) delegating them to the GUI parts. These notifications cannot be sent directly
    to the GUI parts since can be sent from another thread/fiber.
*/
class TConnectionProcessor : public IMailProcessor,
                             public IConnectionStatusDataSource,
                             public IAuthProcessor,
                             protected bts::application_delegate

  {
  public:
    TConnectionProcessor(IGuiUpdateSink& updateSink, const bts::profile_ptr& loadedProfile);
    virtual ~TConnectionProcessor();

  /// IMailProcessor interface implementation:
    /// \see IMailProcessor interface description.
    virtual void Send(const TIdentity& senderId, const TPhysicalMailMessage& msg,
      const TMsgType msg_type, const TStoredMailMessage* savedDraftMsg) override;
    /// \see IMailProcessor interface description.
    virtual TStoredMailMessage Save(const TIdentity& senderId, const TPhysicalMailMessage& srcMsg,
      const TMsgType msg_type, const TStoredMailMessage* msgBeingReplaced) override;

  /// IConnectionStatusDataSource interface implementation:
    /// \see IConnectionStatusDataSource interface description.
    virtual unsigned int GetPeerConnectionCount() const override;
    /// \see IConnectionStatusDataSource interface description.
    virtual bool IsMailConnected() const override;

  /// IAuthProcessor interface implementation:
    /// \see IAuthProcessor interface description.
    virtual void SendAuth(const TCurrIdentity& senderId, const TRequestMessage& msg) override;
    /// \see IAuthProcessor interface description.
    virtual void storeAuthorization(const TCurrIdentity& senderId, const TRequestMessage& src_msg,
      const TStoredMessage& msg_header) override;

  /// Other implementation helpers:

    /** If any message transmission is in progress asks user to stop it.
        \param canBreak - output, optional parameter. If not null will be set to true if transfer
               operation being in progress can be safely cancelled.
        Returns true if application exit can be continued, false otherwise.
    */
    bool CanQuit(bool* canBreak = nullptr) const;

    /** Allows to cancel current mail transmission, ie before quiting the app.
        \warning Can be used only when CanQuit returned true as canBreak parameter.
    */
    void CancelTransmission();

    void updateOptions();

  /// application_delegate interface implementation:
  private:
    /// \see application_delegate interface description.
    virtual void connection_count_changed(unsigned int count) override;
    /// \see application_delegate interface description.
    virtual bool receiving_mail_message() override;
    /// \see application_delegate interface description.
    virtual void received_text(const bts::bitchat::decrypted_message& msg) override;
    /// \see application_delegate interface description.
    virtual void received_email(const bts::bitchat::decrypted_message& msg) override;
    /// \see application_delegate interface description.
    virtual void received_request( const bts::bitchat::decrypted_message& msg) override;
    /// \see application_delegate interface description.
    virtual void received_unsupported_msg( const bts::bitchat::decrypted_message& msg) override;
    /// \see application_delegate interface description.
    virtual void message_transmission_finished(bool success) override;

  /// Other implementation helpers:
  private:
    bool isMyIdentity(const TRecipientPublicKey& senderId);
    typedef bts::bitchat::decrypted_message TStorableMessage;
    void PrepareStorableMessage(const TIdentity& senderId, const TPhysicalMailMessage& msg,
      TStorableMessage* storableMsg);
    void PrepareStorableAuthMsg(const TIdentity& senderId, const TRequestMessage& sourceMsg,
      TStorableMessage* storableMsg);

  /// Class attributes:
  private:
    class TOutboxQueue;
    class TThreadSafeGuiNotifier;

    typedef bts::bitchat::message_db_ptr TMessageDB;
    typedef std::shared_ptr<bts::application> TApplicationPtr;

    /// Sink to notify client about performed operations..
    bts::profile_ptr        Profile;
    TMessageDB              Drafts;
    TOutboxQueue*           OutboxQueue;
    TThreadSafeGuiNotifier* Sink;
    TApplicationPtr         App;
    /// Set to true after CancelTransmission call.
    std::atomic<bool>       TransmissionCancelled;
    /// Set to true between calls: receiving_mail_message <=> received_message
    std::atomic<bool>       ReceivingMail;
    bool                    _chat_allow_flag;
    bool                    _mail_allow_flag;
    bool                    _save_spam_flag;
  };

#endif /// __CONNECTIONPROCESSOR_HPP

