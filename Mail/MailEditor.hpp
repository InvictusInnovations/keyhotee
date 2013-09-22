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
#include <QPushButton>

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
    MailEditor(QWidget *parent = nullptr, QCompleter* contact_completer = nullptr );

Q_SIGNALS:
    void  saveDraft( const DraftMessage& message );
    void  sendMessage( const DraftMessage& message );

protected:
    virtual void closeEvent(QCloseEvent *e);

private:
    void setupMailActions();
    void setupEditActions();
    void setupTextActions();
    bool load(const QString &f);
    bool maybeSave();
    void setCurrentFileName(const QString &fileName);
    void enableSendMoney(bool);
    void showAttachFileDialog(bool);

private slots:
    void moneyUnitChanged(int index);
    void enableFormat(bool show_format );
    bool fileSave();
    bool fileSaveAs();
    void filePrint();
    void filePrintPreview();
    void filePrintPdf();

    void textBold();
    void textUnderline();
    void textItalic();
    void textFamily(const QString &f);
    void textSize(const QString &p);
    void textStyle(int styleIndex);
    void textColor();
    void textAlign(QAction *a);

    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();

    void clipboardDataChanged();
    void about();
    void printPreview(QPrinter *);

    void subjectChanged( const QString& subject );

private:
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);
    void alignmentChanged(Qt::Alignment a);
    void setupMoneyToolBar();

    void setupAddressBar();
    void updateAddressBarLayout();

    QWidget*      address_bar;
    ContactListEdit*  to_field; 
    ContactListEdit*  cc_field;
    ContactListEdit*  bcc_field;
    QTextDocument*    to_values;
    QTextDocument*    cc_values;
    QTextDocument*    bcc_values;
    QLineEdit*    subject_field;
    QComboBox*    from_field;
    QFormLayout*  address_layout;

    QGridLayout*  layout;

    QAction*     actionToggleCc;
    QAction*     actionToggleBcc;
    QAction*     actionToggleFrom;
    QMenu*       fieldsMenu;
    QToolButton* fieldsButton;

    QLineEdit* money_amount;
    QComboBox* money_unit;
    QLabel*    money_balance;

    QAction *actionSave;
    QAction *actionTextBold;
    QAction *actionTextUnderline;
    QAction *actionTextItalic;
    QAction *actionTextColor;
    QAction *actionAlignLeft;
    QAction *actionAlignCenter;
    QAction *actionAlignRight;
    QAction *actionAlignJustify;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionAttachMoney;
    QAction *actionAttachFile;

    QComboBox *comboStyle;
    QFontComboBox *comboFont;
    QComboBox *comboSize;

    QToolBar* format_tb;
    QToolBar* money_tb;
    //QToolBar* style_tb;

    //QToolBar *tb;
    QString fileName;
    QTextEdit *textEdit;

    QCompleter* _contact_completer;
};

