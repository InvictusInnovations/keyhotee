#include "maileditorwindow.hpp"

#include "ui_maileditorwindow.h"

#include "mailfieldswidget.hpp"

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

  MailFields = new MailFieldsWidget(*this);
  ui->mailFieldsToolBar->addWidget(MailFields);

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

  setWindowModified(ui->messageEdit->document()->isModified());
  ui->actionSave->setEnabled(ui->messageEdit->document()->isModified());
  ui->actionUndo->setEnabled(ui->messageEdit->document()->isUndoAvailable());
  ui->actionRedo->setEnabled(ui->messageEdit->document()->isRedoAvailable());

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
  if (a & Qt::AlignLeft)
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
  }

void MailEditorMainWindow::onTextAlignTriggered(QAction *a)
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
  }

void MailEditorMainWindow::onTextBoldTrigerred()
  {
  QTextCharFormat fmt;
  fmt.setFontWeight(ui->actionBold->isChecked() ? QFont::Bold : QFont::Normal);
  mergeFormatOnWordOrSelection(fmt);
  }

void MailEditorMainWindow::onTextUnderlineTrigerred()
  {
  QTextCharFormat fmt;
  fmt.setFontUnderline(ui->actionUnderline->isChecked());
  mergeFormatOnWordOrSelection(fmt);
  }

void MailEditorMainWindow::onTextItalicTrigerred()
  {
  QTextCharFormat fmt;
  fmt.setFontItalic(ui->actionItalic->isChecked());
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

void MailEditorMainWindow::onFileAttachementTriggered()
  {
  }

void MailEditorMainWindow::onMoneyAttachementTriggered()
  {
  }

void MailEditorMainWindow::onSendTriggered()
  {
  }

