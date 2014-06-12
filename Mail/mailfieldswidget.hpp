#ifndef MAILFIELDSWIDGET_HPP
#define MAILFIELDSWIDGET_HPP

#include "ch/mailprocessor.hpp"
#include <bts/addressbook/contact.hpp>
#include "Identity/IdentitiesUpdate.hpp"

#include <QWidget>
#include <map>

namespace Ui 
{
class MailFieldsWidget;
}

class AddressBookModel;

class MailFieldsWidget : public QWidget,
                         protected IIdentitiesUpdate
  {
  Q_OBJECT

  public:
    typedef IMailProcessor::TPhysicalMailMessage TPhysicalMailMessage;
    typedef IMailProcessor::TRecipientPublicKey  TRecipientPublicKey;
    typedef IMailProcessor::TRecipientPublicKeys TRecipientPublicKeys;

    /// Helper enum type to check which fields are visible.
    enum TVisibleFields
      {
      FROM_FIELD  = 0x1,
      CC_FIELD    = 0x2,
      BCC_FIELDS  = 0x4
      };

    MailFieldsWidget(QWidget& parent, QAction& actionSend, AddressBookModel& abModel, bool editMode);
    virtual ~MailFieldsWidget();

    /** Allows to explicityly fill recipient lists with given values.
        For each nonempty optional lists, controls related to them (ie ccList) will be displayed
        automatically.
    */
    void SetRecipientList(const TRecipientPublicKey& senderPK, const TRecipientPublicKeys& toList,
      const TRecipientPublicKeys& ccList, const TRecipientPublicKeys& bccList);

    /// Allows to explicitly set subject.
    void SetSubject(const std::string& subject);
    void SetSubject(const QString& subject);

    /** Loads 'to', 'cc' lists into proper controls.
        Also initializes subject with this one stored in the srcMsg.
    */
    void LoadContents(const TRecipientPublicKey& senderPK, const TPhysicalMailMessage& srcMsg);

    void showFromControls(bool show);
    void showCcControls(bool show);
    void showBccControls(bool show);

    /// Returns currently set subject text.
    QString getSubject() const;
    /// Returns an identity of currently selected sender.
    const bts::addressbook::wallet_identity& GetSenderIdentity() const
      {
      return SenderIdentity;
      }

    void FillRecipientLists(TRecipientPublicKeys* toList, TRecipientPublicKeys* ccList,
      TRecipientPublicKeys* bccList) const;

    /// Allows to check which field is visible.
    bool isFieldVisible(TVisibleFields field) const
      {
      return (VisibleFields & field) != 0;
      }

    /// Calling from maileditorwindow when window get closeEvent
    void closeEvent();

  Q_SIGNAL void subjectChanged(const QString& subject);
  Q_SIGNAL void recipientListChanged();

  protected:
    /// \see IIdentitiesUpdate interface description.
    virtual void onIdentitiesChanged(const TIdentities& identities) override;
    virtual bool onIdentityDelIntent(const TIdentity&  identity) override { return true; }
    virtual bool onIdentityDelete(const TIdentity&  identity) override { return true; }

  private:
    /** Helper function to trim subject text and avoid crashes since looks like QT Line edit has
        a bug and doesn't apply a limit configured to it.
    */
    QString trimSubject(const QString& subject) const;
    /// Allows to show/hide given layout & all widgets associated with it.
    void showChildLayout(QLayout* layout, bool show, int preferredPosition, unsigned int fieldFlag);
    /// Helper for showChildLayout.
    void showLayoutWidgets(QLayout* layout, bool show);
    void validateSendButtonState();
    void fillSenderIdentities();
    /** Allows to select identity with given public key as current one.
        \warning This method can be used in 2 contexts:
        - in context of active edit mode, when specified senderPK should point to one of registered
          identities,
        - in context of active read-only mode, when specified senderPK can point to any known
          contact or even unknown one (then just public key should be displayed).
    */
    void selectSenderIdentity(const TRecipientPublicKey& senderPK);
    void setChosenSender(const TRecipientPublicKey& senderPK);

  private slots:
    void on_sendButton_clicked();
    void onRecipientListChanged();
    void onSubjectChanged(const QString& subject);
    void onFromBtnTriggered(QAction* action);

  private:
    typedef std::map<QAction*, IMailProcessor::TIdentity> TAction2IdentityIndex;
    Ui::MailFieldsWidget*             ui;
    QAction&                          ActionSend;
    unsigned int                      VisibleFields;
    /// Helper map to associate action for created 'from-sub-menu' item to given wallet_identity.
    TAction2IdentityIndex             Action2Identity;
    /// Can store selected wallet_identity or just one of known contacts...
    IMailProcessor::TIdentity         SenderIdentity;
    bool                              EditMode;
  };

#endif // MAILFIELDSWIDGET_HPP
