#ifndef __MAILEDITORWINDOW_HPP
#define __MAILEDITORWINDOW_HPP

#include "ch/mailprocessor.hpp"

#include <bts/bitchat/bitchat_message_db.hpp>

#include <QMainWindow>

#include <utility>

namespace Ui { class MailEditorWindow; }

class QComboBox;
class QColor;
class QFont;
class QFontComboBox;
class QModelIndex;
class QTextCharFormat;

class AddressBookModel;
class TFileAttachmentWidget;
class MailFieldsWidget;
class TMoneyAttachementWidget;

/** Mail message editor/viewer window.
    Contains rich editor, file attachment browser, money attachment browser.
    Can be spawned:
    - without any recipient specified when new message is created without recipient context
    - with explicit recipient list when new message is created in context of selected recipient
      (contact)
    - with explicit recipient list when reply/reply-all options are in action.
*/
class MailEditorMainWindow : public QWidget
  {
  Q_OBJECT
  public:
    typedef IMailProcessor::TPhysicalMailMessage TPhysicalMailMessage;
    typedef IMailProcessor::TRecipientPublicKey  TRecipientPublicKey;
    typedef IMailProcessor::TRecipientPublicKeys TRecipientPublicKeys;
    typedef IMailProcessor::TStoredMailMessage   TStoredMailMessage;

    MailEditorMainWindow(QWidget* parent, AddressBookModel& abModel, IMailProcessor& mailProcessor,
      bool editMode);
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
    */
    void LoadMessage(const TStoredMailMessage& srcMsgHeader, const TPhysicalMailMessage& srcMsg);

  private:
    /// QWidget reimplementation to support query for save mod. contents.
    virtual void closeEvent(QCloseEvent* e) override;
    
  /// Other helper methods:
    bool maybeSave();
    void setupEditorCommands();
    /// Updates UI status regarding to chosen alignment.
    void alignmentChanged(Qt::Alignment a);
    void fontChanged(const QFont& f);
    void colorChanged(const QColor& c);
    void mergeFormatOnWordOrSelection(const QTextCharFormat& format);
    /// Returns true if preparation succeeded or false if not.
    bool prepareMailMessage(TPhysicalMailMessage* storage);
    /// Loads given message contents into all editor controls.
    void loadContents(const TRecipientPublicKey& senderId, const TPhysicalMailMessage& srcMsg);
    void toggleReadOnlyMode();

  private slots:
    /// Actual implementation of save operation.
    void onSave();
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

  private:
    /** pairs loaded encoded draft message & flag determining it was specified (it is impossible
        to query TStoredMailMessage for 'valid' property.
    */
    typedef std::pair<TStoredMailMessage, bool> TDraftMessageInfo;

    Ui::MailEditorWindow*    ui;
    /** Filled when mail editor has been opened with message already stored in Drafts. During save
        this old message should be replaced with new one.
    */
    TDraftMessageInfo        DraftMessageInfo;
    AddressBookModel&        ABModel;
    IMailProcessor&          MailProcessor;
    MailFieldsWidget*        MailFields;
    TMoneyAttachementWidget* MoneyAttachement;
    TFileAttachmentWidget*   FileAttachment;
    QFontComboBox*           FontCombo;
    QComboBox*               FontSize;
    bool                     EditMode;
  };

#endif ///__MAILEDITORWINDOW_HPP


