#ifndef __MAILEDITORWINDOW_HPP
#define __MAILEDITORWINDOW_HPP

#include <QMainWindow>

namespace Ui { class MailEditorWindow; }

class QColor;
class QFont;
class QTextCharFormat;

class MailFieldsWidget;

class MailEditorMainWindow : public QMainWindow
  {
  Q_OBJECT
  public:
    MailEditorMainWindow(QWidget* parent = nullptr);
    virtual ~MailEditorMainWindow();

  private:
    /// QWidget reimplementation to support query for save mod. contents.
    virtual void closeEvent(QCloseEvent* e) override;
    
  /// Other helper methods:
    bool maybeSave();
    void setupEditorCommands();
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
    void onTextAlignTriggered(QAction* a);
    void onTextBoldTrigerred();
    void onTextUnderlineTrigerred();
    void onTextItalicTrigerred();

  /// Mail receipents controls selection:
    void onCcTriggered(bool checked);
    void onBccTriggered(bool checked);
    void onFromTriggered(bool checked);

    void onFileAttachementTriggered();
    void onMoneyAttachementTriggered();
    /// Allows to send mail document prepared in current window.
    void on_actionSend_triggered();

  private:
    Ui::MailEditorWindow* ui;
    MailFieldsWidget*     MailFields;
  };

#endif ///__MAILEDITORWINDOW_HPP


