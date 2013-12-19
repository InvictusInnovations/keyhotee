#ifndef __MAILEDITORWINDOW_HPP
#define __MAILEDITORWINDOW_HPP

#include "ch/mailprocessor.hpp"

#include <QMainWindow>

namespace Ui { class MailEditorWindow; }


class QComboBox;
class QColor;
class QFont;
class QFontComboBox;
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
class MailEditorMainWindow : public QMainWindow
  {
  Q_OBJECT
  public:
    typedef IMailProcessor::TRecipientPublicKeys TRecipientPublicKeys;

    MailEditorMainWindow(QWidget* parent, AddressBookModel& abModel, IMailProcessor& mailProcessor,
      bool editMode);
    virtual ~MailEditorMainWindow();

    /** Allows to explicitly specify initial recipient lists while creating a mail window.
    */
    void SetRecipientList(const TRecipientPublicKeys& toList, const TRecipientPublicKeys& ccList,
      const TRecipientPublicKeys& bccList);

  private:
    typedef IMailProcessor::TPhysicalMailMessage TPhysicalMailMessage;

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
    /// Returns true if preparation succeeded or not.
    bool prepareMailMessage(TPhysicalMailMessage* storage, TRecipientPublicKeys* bccList);

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

    void onFileAttachementTriggered(bool checked);
    void onMoneyAttachementTriggered(bool checked);
    /// Allows to send mail document prepared in current window.
    void on_actionSend_triggered();
    /// Notification for subject changes needed to update window title.
    void onSubjectChanged(const QString& subject);

  private:
    Ui::MailEditorWindow*    ui;
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


