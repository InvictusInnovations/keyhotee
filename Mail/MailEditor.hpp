#pragma once 

#include <QMainWindow>
#include <QMap>
#include <QPointer>
#include <QDialog>
#include <QGridLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QWidgetAction>
#include <QTableWidget>
#include <fc/crypto/elliptic.hpp>
#include <bts/application.hpp>

QT_BEGIN_NAMESPACE
class QAction;
class QComboBox;
class QFontComboBox;
class QTextEdit;
class QTextCharFormat;
class QMenu;
class QPrinter;
class QLabel;
QT_END_NAMESPACE

class ContactListEdit;
class DraftMessage;


class MailEditor : public QDialog
{
    Q_OBJECT

public:
          MailEditor(QWidget* parent = nullptr);

   static void setContactCompleter(QCompleter* completer) { _contact_completer = completer; }

    void  setFocusAndShow();
    void  addToContact(int contact_id);
    void  addToContact(fc::ecc::public_key public_key);

    void  addCcContact(int contact_id);
    void  addCcContact(fc::ecc::public_key public_key);

    void  copyToBody(QString body_string);
    void  SetSubject(QString subject_string);


Q_SIGNALS:
    void  saveDraft( const DraftMessage& message );
    void  sendMessage( const DraftMessage& message );

protected:
    virtual void closeEvent(QCloseEvent* close_event);

private:
    void setupMailActions();
    void setupEditActions();
    void setupTextActions();
    bool load(const QString& fileName);
    bool maybeSave();
    void setCurrentFileName(const QString& fileName);
    void enableSendMoney(bool);
    void showAttachFileDialog(bool);


private slots:
    void moneyUnitChanged(int index);
    void enableFormat(bool show_format );
    void sendMailMessage();
    bool fileSave();
    bool fileSaveAs();
    void filePrint();
    void filePrintPreview();
    void filePrintPdf();

    void textBold();
    void textUnderline();
    void textItalic();
    void textFamily(const QString& family);
    void textSize(const QString& point_size);
    void textStyle(int style_index);
    void textColor();
    void textAlign(QAction* a);

    void currentCharFormatChanged(const QTextCharFormat& format);
    void cursorPositionChanged();

    void clipboardDataChanged();
    void about();
    void printPreview(QPrinter*);

    void subjectChanged( const QString& subject );

    void showContextMenu(const QPoint& p);
    void removeAttachments();
    void renameAttachment();
    void setModifiedFilenames();
    void selectAllAttachment();
    void setupAttachmentTable();

private:
    void mergeFormatOnWordOrSelection(const QTextCharFormat& format);
    void fontChanged(const QFont& new_font);
    void colorChanged(const QColor& new_color);
    void alignmentChanged(Qt::Alignment alignment);
    void setupMoneyToolBar();

    void setupAddressBar();

    void attachFile(QString filename);
    void updateAddressBarLayout();

    QWidget*      address_bar;

    QTableWidget* _attachment_table;
    QHBoxLayout*       _hbox_layout;
    QStringList     _selected_files;

    ContactListEdit*  to_field; 
    ContactListEdit*  cc_field;
    ContactListEdit*  bcc_field;
    QTextDocument*    to_values;
    QTextDocument*    cc_values;
    QTextDocument*    bcc_values;
    QLineEdit*    subject_field;
    QComboBox*    from_field;
    QFormLayout*  address_layout;

    std::vector<bts::bitchat::attachment>        _attachments;
    std::vector<std::string>               _absolute_filename;
    std::vector<qint64>                         _sizeof_files;

    QMenu*            _contextMenu;
    QAction*          _attach_file;
    QAction*    _remove_attachment;
    QAction*    _rename_attachment;
    QAction* _selectall_attachment;

    QGridLayout*  layout;

    QAction*     actionToggleCc;
    QAction*     actionToggleBcc;
    QAction*     actionToggleFrom;
    QMenu*       fieldsMenu;
    QToolButton* fieldsButton;

    QLineEdit* money_amount;
    QComboBox* money_unit;
    QLabel*    money_balance;

    QAction* actionSave;
    QAction* actionTextBold;
    QAction* actionTextUnderline;
    QAction* actionTextItalic;
    QAction* actionTextColor;
    QAction* actionAlignLeft;
    QAction* actionAlignCenter;
    QAction* actionAlignRight;
    QAction* actionAlignJustify;
    QAction* actionUndo;
    QAction* actionRedo;
    QAction* actionCut;
    QAction* actionCopy;
    QAction* actionPaste;
    QAction* actionAttachMoney;
    QAction* actionAttachFile;

    QComboBox *comboStyle;
    QFontComboBox *comboFont;
    QComboBox *comboSize;

    QToolBar* format_tool_bar;
    QToolBar* money_tool_bar;
    //QToolBar* style_tb;

    //QToolBar *tool_bar;
    QString _fileName;
    QTextEdit* textEdit;

    static QCompleter* _contact_completer;
};

