#ifndef __MAILEDITORWINDOW_HPP
#define __MAILEDITORWINDOW_HPP

#include "ch/mailprocessor.hpp"

#include <bts/bitchat/bitchat_message_db.hpp>

#include "ATopLevelWindow.hpp"
#include "BlockerDelegate.hpp"

#include <QMainWindow>

namespace Ui { class MailEditorWindow; }

class QComboBox;
class QColor;
class QFont;
class QFontComboBox;
class QModelIndex;
class QTextCharFormat;

class AddressBookModel;
class Contact;
class TFileAttachmentWidget;
class MailFieldsWidget;
class TMoneyAttachementWidget;
class Mailbox;



/** Mail message editor/viewer window.
    Contains rich editor, file attachment browser, money attachment browser.
    Can be spawned:
    - without any recipient specified when new message is created without recipient context
    - with explicit recipient list when new message is created in context of selected recipient
      (contact)
    - with explicit recipient list when reply/reply-all options are in action.
*/
class MailEditorMainWindow : public ATopLevelWindow,
                             protected IBlockerDelegate
  {
  Q_OBJECT
  public:
    typedef IMailProcessor::TPhysicalMailMessage TPhysicalMailMessage;
    typedef IMailProcessor::TRecipientPublicKey  TRecipientPublicKey;
    typedef IMailProcessor::TRecipientPublicKeys TRecipientPublicKeys;
    typedef IMailProcessor::TStoredMailMessage   TStoredMailMessage;

    /// Determines way how to source message should be duplicated.
    enum TLoadForm
      {
      /// Message is loaded as a draft - no any changes is needed
      Draft,
      /// Allows to create a forwarded message (\see description of LoadMessage)
      Forward,
      /// Allows to create an answer message (\see description of LoadMessage)
      Reply,
      /// Allows to create an answer message (\see description of LoadMessage)
      ReplyAll
      };

    MailEditorMainWindow(ATopLevelWindowsContainer* parent, AddressBookModel& abModel,
      IMailProcessor& mailProcessor, bool editMode);
    virtual ~MailEditorMainWindow();

    /** Allows to explicitly specify initial recipient lists while creating a mail window.
        Useful for instantiating mail editor with explicit specified recipient.
    */
    void SetRecipientList(const TRecipientPublicKeys& toList, const TRecipientPublicKeys& ccList,
      const TRecipientPublicKeys& bccList);

    /** Allows to load message contents into mail window.
        \param srcMsgHeader - original backend object (got directly from mail_db) holding encoded
                              form of message data.
                              Needed to notify mail processor than again saved message should
                              replace given one than creating another object in storage.
        \param srcMsg       - decoded (from srcMsgHeader) backend object holding saved/received
                              message data.
        \param loadForm     - determines how original message contents should be transformed.

        Message have to been modified to match 'forwarded/replied' message specification:
          - in the message text an additional header should be added containing original sender,
            date, subject info. 
          - attachment list should be cleared in case of Reply/ReplyAll forms
          - new recipient list should be modified regarding to chosen load form:
            a) Forward  - recipient list should be left empty.
            b) Reply    - only original sender should be added as 'to' recipient
            c) ReplyAll - 'to' list should contain original sender and original recipient list
                          (except our own identity).
                          'cc' list should contain others contacts originally placed on 'cc' list
                          if they are not yet on 'to' list.
    */
    void LoadMessage(Mailbox* mailbox, const TStoredMailMessage& srcMsgHeader, const TPhysicalMailMessage& srcMsg,
      TLoadForm loadForm);
    void addContactCard (const Contact&);

  private:
    /// QWidget reimplementation to support query for save mod. contents.
    virtual void closeEvent(QCloseEvent* e) override;
    
  /// Other helper methods:
    /** Dedicated to check if editor contents should be saved before closing. All checks against
        save conditions should be rather done here instead of onSave method, even there is good
        resoning for that (like message size checking).
    */
    bool maybeSave();
    void setupEditorCommands();
    /** Allows to check given message size against limit set by application. 
        Returns true on success, false when limit has been exceeded - also reports a warning message
        in this case.
    */
    bool checkMsgSize(const TPhysicalMailMessage& srcMsg);
    /// Updates UI status regarding to chosen alignment.
    void alignmentChanged(Qt::Alignment a);
    void fontChanged(const QFont& f);
    void colorChanged(const QColor& c);
    void mergeFormatOnWordOrSelection(const QTextCharFormat& format);
    /// Returns true if preparation succeeded or false if not.
    bool prepareMailMessage(TPhysicalMailMessage* storage);
    /// Loads given message contents into all editor controls.
    void loadContents(const TRecipientPublicKey& senderId, const TPhysicalMailMessage& srcMsg);
    /** Allows to transform source recipient list to properly fill new one.
    */
    void transformRecipientList(const TRecipientPublicKey& senderId,
      const TRecipientPublicKeys& sourceToList, const TRecipientPublicKeys& sourceCCList);
    /** Allows to transform original message body to the 'answered' form (with additional header
        containing original sender info etc).
        Next fills editor window with such transformed body.
    */
    QString transformMailBody(TLoadForm loadForm, const TStoredMailMessage& msgHeader,
      const TPhysicalMailMessage& srcMsg);
    void toggleReadOnlyMode();

  private:
    /// IBlockerDelegate interface description:
    /// \see IBlockerDelegate interface description.
    virtual void onBlockedImage() override;
    /// \see IBlockerDelegate interface description.
    virtual void onLoadBlockedImages() override;

  private slots:
    /// Actual implementation of save operation.
    bool onSave();
    /// Needed to properly update 'paste' controls.
    void onClipboardDataChanged();

  /// Cursor positioning slots to update text formatting controls
    void onCurrentCharFormatChanged(const QTextCharFormat& format);
    void onCursorPositionChanged();

  /// Text formatting slots:
    void onTextAlignTriggerred(QAction* a);
    void onTextBoldTriggerred(bool checked);
    void onTextUnderlineTriggerred(bool checked);
    void onTextItalicTriggerred(bool checked);
    void onTextColorTriggerred();
    void onTextFamilyChanged(const QString &f);
    void onTextSizeChanged(const QString &p);

  /// Mail receipents controls selection:
    void onCcTriggered(bool checked);
    void onBccTriggered(bool checked);
    void onFromTriggered(bool checked);

    void onShowFormattingControlsTriggered(bool checked);
    void onFileAttachementTriggered(bool checked);
    void onMoneyAttachementTriggered(bool checked);
    /// Allows to send mail document prepared in current window.
    void on_actionSend_triggered();
    /// Notification for subject changes needed to update window title.
    void onSubjectChanged(const QString& subject);
    /// Notification from recipient list changes to update document modified state.
    void onRecipientListChanged();
    /// Notification from attachment list widget about attachment list changes.
    void onAttachmentListChanged();
    /// Notification sent when some file attachement item(s) will be pasted/dropeed onto document body.
    void onFileAttachmentAdded(const QStringList& files);

  /// Mail Reply/Forward
    void onMailReplyTriggered();
    void onMailReplyAllTriggered();
    void onMailForwardTriggered();

  private:
    Ui::MailEditorWindow*    ui;
    /** Filled when mail editor has been opened with message already stored in Drafts. During save
        this old message should be replaced with new one.
    */
    fc::optional<TStoredMailMessage>  DraftMessage;
    AddressBookModel&                 ABModel;
    IMailProcessor&                   MailProcessor;
    MailFieldsWidget*                 MailFields;
    TMoneyAttachementWidget*          MoneyAttachement;
    TFileAttachmentWidget*            FileAttachment;
    QFontComboBox*                    FontCombo;
    QComboBox*                        FontSize;
    bool                              EditMode;
    fc::optional<fc::uint256>         _src_msg_id;
    IMailProcessor::TMsgType          _msg_type;
    ATopLevelWindowsContainer*        _parent;
    TPhysicalMailMessage              _src_msg;
  };

#endif ///__MAILEDITORWINDOW_HPP


