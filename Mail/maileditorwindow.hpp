#ifndef __MAILEDITORWINDOW_HPP
#define __MAILEDITORWINDOW_HPP

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

class MailEditorMainWindow : public QMainWindow
  {
  Q_OBJECT
  public:
    MailEditorMainWindow(QWidget* parent, AddressBookModel& abModel);
    virtual ~MailEditorMainWindow();

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
    MailFieldsWidget*        MailFields;
    TMoneyAttachementWidget* MoneyAttachement;
    TFileAttachmentWidget*   FileAttachment;
    QFontComboBox*           FontCombo;
    QComboBox*               FontSize;
  };

#endif ///__MAILEDITORWINDOW_HPP


