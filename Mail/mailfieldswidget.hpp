#ifndef MAILFIELDSWIDGET_HPP
#define MAILFIELDSWIDGET_HPP

#include "ch/mailprocessor.hpp"

#include <bts/addressbook/contact.hpp>

#include <QWidget>
#include <map>

namespace Ui 
{
class MailFieldsWidget;
}

class AddressBookModel;

class MailFieldsWidget : public QWidget
  {
  Q_OBJECT

  public:
    typedef IMailProcessor::TPhysicalMailMessage TPhysicalMailMessage;
    typedef IMailProcessor::TRecipientPublicKey  TRecipientPublicKey;
    typedef IMailProcessor::TRecipientPublicKeys TRecipientPublicKeys;

    MailFieldsWidget(QWidget& parent, QAction& actionSend, AddressBookModel& abModel);
    virtual ~MailFieldsWidget();

    /** Allows to explicityly fill recipient lists with given values.
        For each nonempty optional lists, controls related to them (ie ccList) will be displayed
        automatically.
    */
    void SetRecipientList(const TRecipientPublicKeys& toList, const TRecipientPublicKeys& ccList,
      const TRecipientPublicKeys& bccList);

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

  Q_SIGNAL void subjectChanged(const QString& subject);
  Q_SIGNAL void recipientListChanged();

  private:
    enum TVisibleFields
      {
      FROM_FIELD  = 0x1,
      CC_FIELD    = 0x2,
      BCC_FIELDS  = 0x4
      };

    /// Allows to show/hide given layout & all widgets associated with it.
    void showChildLayout(QLayout* layout, bool show, int preferredPosition, unsigned int fieldFlag);
    /// Helper for showChildLayout.
    void showLayoutWidgets(QLayout* layout, bool show);
    void validateSendButtonState();
    void fillSenderIdentities();
    /// Allows to select identity with given public key as current one.
    void selectSenderIdentity(const TRecipientPublicKey& senderPK);
    bool isFieldVisible(TVisibleFields field) const;

  private slots:
    void on_sendButton_clicked();
    void onRecipientListChanged();
    void onSubjectChanged(const QString& subject);
    void onFromBtnTriggered(QAction* action);

  private:
    typedef std::map<QAction*, bts::addressbook::wallet_identity> TAction2IdentityIndex;
    Ui::MailFieldsWidget*             ui;
    QAction&                          ActionSend;
    unsigned int                      VisibleFields;
    /// Helper map to associate action for created 'from-sub-menu' item to given wallet_identity.
    TAction2IdentityIndex             Action2Identity;
    bts::addressbook::wallet_identity SenderIdentity;
  };

#endif // MAILFIELDSWIDGET_HPP
