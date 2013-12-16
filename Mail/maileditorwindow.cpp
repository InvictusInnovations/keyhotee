#include "maileditorwindow.hpp"

#include "ui_maileditorwindow.h"

#include "mailfieldswidget.hpp"
#include "moneyattachementwidget.hpp"

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QToolBar>
#include <QToolButton>

MailEditorMainWindow::MailEditorMainWindow(QWidget* parent /*= nullptr*/) :
  QMainWindow(parent, Qt::WindowFlags(Qt::WindowType::Dialog)),
  ui(new Ui::MailEditorWindow())
  {
  ui->setupUi(this);

  /** Disable these toolbars by default. They should be showed up on demand, when given action will
      be trigerred.
  */
  ui->fileAttachementToolBar->hide();

  ui->moneyAttachementToolBar->hide();
  MoneyAttachement = new TMoneyAttachementWidget(ui->moneyAttachementToolBar);
  ui->moneyAttachementToolBar->addWidget(MoneyAttachement);

  MailFields = new MailFieldsWidget(*this, *ui->actionSend);

  /// Initially only basic mail fields (To: and Subject:) should be visible
  MailFields->showFromControls(false);
  MailFields->showCcControls(false);
  MailFields->showBccControls(false);

  ui->mailFieldsToolBar->addWidget(MailFields);

  connect(MailFields, SIGNAL(subjectChanged(QString)), this, SLOT(onSubjectChanged(QString)));

  /** Supplement definition of mailFieldSelectorToolbar since Qt Creator doesn't support putting
      into its context dedicated controls (like preconfigured toolbutton).

      Setup local menu for 'actionMailFields' toolButton (used to enable/disable additional mail
      field selection).
  */
  QMenu* mailFieldsMenu = new QMenu(this);
  mailFieldsMenu->addAction(ui->actionFrom);
  mailFieldsMenu->addAction(ui->actionCC);
  mailFieldsMenu->addAction(ui->actionBCC);

  ui->actionMailFields->setMenu(mailFieldsMenu);
  ui->mailFieldSelectorToolBar->addAction(ui->actionMailFields);

  setupEditorCommands();

  ui->messageEdit->setFocus();
  fontChanged(ui->messageEdit->font());
  colorChanged(ui->messageEdit->textColor());
  alignmentChanged(ui->messageEdit->alignment());

  QString subject = MailFields->getSubject();
  onSubjectChanged(subject);

  setWindowModified(ui->messageEdit->document()->isModified());
  ui->actionSave->setEnabled(ui->messageEdit->document()->isModified());
  ui->actionUndo->setEnabled(ui->messageEdit->document()->isUndoAvailable());
  ui->actionRedo->setEnabled(ui->messageEdit->document()->isRedoAvailable());

  /// Setup command update ui related to 'save' option activity control and window modify marker.
  connect(ui->messageEdit->document(), SIGNAL(modificationChanged(bool)), ui->actionSave,
    SLOT(setEnabled(bool)));
  connect(ui->messageEdit->document(), SIGNAL(modificationChanged(bool)), this,
    SLOT(setWindowModified(bool)));

#ifndef QT_NO_CLIPBOARD
  connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(onClipboardDataChanged()));
#endif
  }

MailEditorMainWindow::~MailEditorMainWindow()
  {
  delete ui;
  }

void MailEditorMainWindow::closeEvent(QCloseEvent *e)
  {
  if(maybeSave())
    e->accept();
  else
    e->ignore();
  }

bool MailEditorMainWindow::maybeSave()
  {
  if(ui->messageEdit->document()->isModified() == false)
      return true;

  QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("Keyhotee"),
    tr("The document has been modified.\nDo you want to save your changes ?"),
    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  
  if(ret == QMessageBox::Save)
    onSave();
  else
  if(ret == QMessageBox::Cancel)
    return false;

  return true;
  }

void MailEditorMainWindow::setupEditorCommands()
  {
  }

void MailEditorMainWindow::alignmentChanged(Qt::Alignment a)
  {
  ui->actionLeft->setChecked(false);
  ui->actionCenter->setChecked(false);
  ui->actionRight->setChecked(false);
  ui->actionJustify->setChecked(false);

  if(a & Qt::AlignLeft)
    ui->actionLeft->setChecked(true);
  else if (a & Qt::AlignHCenter)
    ui->actionCenter->setChecked(true);
  else if (a & Qt::AlignRight)
    ui->actionRight->setChecked(true);
  else if (a & Qt::AlignJustify)
    ui->actionJustify->setChecked(true);
  }

void MailEditorMainWindow::fontChanged(const QFont &f)
  {
//  comboFont->setCurrentIndex(comboFont->findText(QFontInfo(f).family()));
//  comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
  ui->actionBold->setChecked(f.bold());
  ui->actionItalic->setChecked(f.italic());
  ui->actionUnderline->setChecked(f.underline());
  }

void MailEditorMainWindow::colorChanged(const QColor &c)
  {
  QPixmap pix(16, 16);
  pix.fill(c);
  //actionTextColor->setIcon(pix);
  }

void MailEditorMainWindow::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
  {
  QTextCursor cursor = ui->messageEdit->textCursor();
  if(cursor.hasSelection() == false)
    cursor.select(QTextCursor::WordUnderCursor);
  cursor.mergeCharFormat(format);
  ui->messageEdit->mergeCurrentCharFormat(format);
  }

void MailEditorMainWindow::onSave()
  {
  ui->messageEdit->document()->setModified(false);

  /// FIXME do actual save operation
  }

void MailEditorMainWindow::onClipboardDataChanged()
  {
#ifndef QT_NO_CLIPBOARD
  if(const QMimeData *md = QApplication::clipboard()->mimeData())
    ui->actionPaste->setEnabled(md->hasText());
#endif
  }

void MailEditorMainWindow::onCurrentCharFormatChanged(const QTextCharFormat &format)
  {
  fontChanged(format.font());
  colorChanged(format.foreground().color());
  }

void MailEditorMainWindow::onCursorPositionChanged()
  {
  /** When cursor pos. changes update alignment & text formatting controls according to format
      of text under cursor.
  */
  alignmentChanged(ui->messageEdit->alignment());
  QTextCursor cursor = ui->messageEdit->textCursor();
  fontChanged(cursor.charFormat().font());
  }

void MailEditorMainWindow::onTextAlignTriggerred(QAction *a)
  {
  QTextEdit* edit = ui->messageEdit;
  if (a == ui->actionLeft)
    edit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
  else if (a == ui->actionCenter)
    edit->setAlignment(Qt::AlignHCenter);
  else if (a == ui->actionRight)
    edit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
  else if (a == ui->actionJustify)
    edit->setAlignment(Qt::AlignJustify);

  alignmentChanged(edit->alignment());
  }

void MailEditorMainWindow::onTextBoldTriggerred(bool checked)
  {
  QTextCharFormat fmt;
  fmt.setFontWeight(checked ? QFont::Bold : QFont::Normal);
  mergeFormatOnWordOrSelection(fmt);
  }

void MailEditorMainWindow::onTextUnderlineTriggerred(bool checked)
  {
  QTextCharFormat fmt;
  fmt.setFontUnderline(checked);
  mergeFormatOnWordOrSelection(fmt);
  }

void MailEditorMainWindow::onTextItalicTriggerred(bool checked)
  {
  QTextCharFormat fmt;
  fmt.setFontItalic(checked);
  mergeFormatOnWordOrSelection(fmt);
  }

void MailEditorMainWindow::onCcTriggered(bool checked)
  {
  MailFields->showCcControls(checked);
  }

void MailEditorMainWindow::onBccTriggered(bool checked)
  {
  MailFields->showBccControls(checked);
  }

void MailEditorMainWindow::onFromTriggered(bool checked)
  {
  MailFields->showFromControls(checked);
  }

void MailEditorMainWindow::onFileAttachementTriggered(bool checked)
  {
  ui->fileAttachementToolBar->setVisible(checked);
  }

void MailEditorMainWindow::onMoneyAttachementTriggered(bool checked)
  {
  ui->moneyAttachementToolBar->setVisible(checked);
  }

void MailEditorMainWindow::on_actionSend_triggered()
  {
  /// TODO implement actual mail send operation
  }

void MailEditorMainWindow::onSubjectChanged(const QString& subject)
  {
  setWindowTitle(tr("%1[*]").arg(subject));
  }

