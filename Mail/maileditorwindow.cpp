#include "maileditorwindow.hpp"

#include "ui_maileditorwindow.h"

#include "fileattachmentwidget.hpp"
#include "mailfieldswidget.hpp"
#include "moneyattachementwidget.hpp"
#include "utils.hpp"
#include "Mailbox.hpp"

#include <bts/profile.hpp>

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QFontComboBox>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QTextDocumentFragment>
#include <QToolBar>
#include <QToolButton>

namespace
{
typedef IMailProcessor::TRecipientPublicKey TRecipientPublicKey;

class TRecipientPublicKeyLess
  {
  public:
    bool operator()(const TRecipientPublicKey& pk1, const TRecipientPublicKey& pk2) const
      {
      return pk1.serialize() < pk2.serialize();
      }
  };

typedef std::set<TRecipientPublicKey, TRecipientPublicKeyLess> TPublicKeyIndex;
typedef std::pair<TPublicKeyIndex::iterator, bool>             TInsertInfo;
typedef IMailProcessor::TStoredMailMessage                     TStoredMailMessage;
typedef IMailProcessor::TPhysicalMailMessage                   TPhysicalMailMessage;
typedef MailEditorMainWindow::TLoadForm                        TLoadForm;

/** Helper class able to do replied mail document transformations.
*/
class TDocumentTransform
  {
  public:
    QString Do(TLoadForm loadForm, const TStoredMailMessage& msgHeader,
      const TPhysicalMailMessage& srcMsg, QTextDocument* doc);

  private:
    QTextCursor replace(const char* textToFind, const QString& replacement,
      const QTextCursor& startPos = QTextCursor());
    QTextCursor replace(const char* textToFind, const QTextDocumentFragment& replacement,
      const QTextCursor& startPos = QTextCursor());
    /// Allows to remove whole line containing given text.
    void removeContainingLine(const char* textToFind, const QTextCursor& startPos = QTextCursor());
    QTextCursor find(const char* textToFind, const QTextCursor& startPos);
  /// Class attributes:
  private:
    QTextDocument* Doc;
  };

QString
TDocumentTransform::Do(TLoadForm loadForm, const TStoredMailMessage& msgHeader,
  const TPhysicalMailMessage& srcMsg, QTextDocument* doc)
  {
  Doc = doc;

  QString newSubject;
  QString subject = QString::fromStdString(srcMsg.subject);
  QString re = QObject::tr("Re: ");
  QString fwd = QObject::tr("Fwd: ");

  switch(loadForm)
    {
    case TLoadForm::Reply:
    case TLoadForm::ReplyAll:
      if(subject.startsWith(re))
        newSubject = "";
      else
        newSubject = re;
      break;
    case TLoadForm::Forward:
      if(subject.startsWith(fwd))
        newSubject = "";
      else
        newSubject = fwd;
      break;
    case TLoadForm::Draft:
    default:
      assert(false);
    }

  newSubject += srcMsg.subject.c_str();

  QFile htmlPattern(":/Mail/RepliedMailPattern.html");
  if(htmlPattern.open(QFile::ReadOnly) == false)
    {
    /// If pattern cannot be loaded for some reason just load original text :-(
    doc->setHtml(QString(srcMsg.body.c_str()));
    return newSubject;
    }

  QByteArray contents = htmlPattern.readAll();
  QString patternHtml(contents);
  doc->setHtml(patternHtml);

  QString senderText(Utils::toString(msgHeader.from_key, Utils::FULL_CONTACT_DETAILS));
  QString sentDate(Utils::toQDateTime(msgHeader.from_sig_time).toString(Qt::DefaultLocaleShortDate));
  QString toList(Utils::makeContactListString(srcMsg.to_list, ';', Utils::FULL_CONTACT_DETAILS));
  QString ccList(Utils::makeContactListString(srcMsg.cc_list, ';', Utils::FULL_CONTACT_DETAILS));

  replace("$$SENDER$$", senderText);
  replace("$$SENT_DATE$$", sentDate);
  replace("$$TO_RECIPIENTS$$", toList);
  
  if(ccList.isEmpty())
    removeContainingLine("$$CC_RECIPIENTS$$");
  else
    replace("$$CC_RECIPIENTS$$", ccList);

  replace("$$SUBJECT$$", newSubject);

  QTextDocumentFragment tf(QTextDocumentFragment::fromHtml(QString(srcMsg.body.c_str())));
  replace("$$SOURCE_BODY$$", tf);
  
  return newSubject;
  }

inline
QTextCursor TDocumentTransform::replace(const char* textToFind, const QString& replacement,
  const QTextCursor& startPos /*= QTextCursor()*/)
  {
  /** \warning It is impossible to use here QTextDocumentFragment::fromPlainText and next pass
      it to another replace version, since formatting gets broken (new instered text uses formatting
      from begin of block instead of this one which was specified for replaced text).
      It looks like it is some bug in insertFragment (where fragment was built from plain text).
  */
  QTextCursor foundPos = find(textToFind, startPos);

  if(foundPos.isNull() == false && foundPos.hasSelection())
    {
    auto cf = foundPos.charFormat();
    foundPos.beginEditBlock();
    foundPos.removeSelectedText();
    foundPos.insertText(replacement, cf);
    foundPos.endEditBlock();
    }

  return foundPos;
  }

inline
QTextCursor 
TDocumentTransform::replace(const char* textToFind, const QTextDocumentFragment& replacement,
  const QTextCursor& startPos /*= QTextCursor()*/)
  {
  QTextCursor foundPos = find(textToFind, startPos);

  if(foundPos.isNull() == false && foundPos.hasSelection())
    {
    foundPos.beginEditBlock();
    foundPos.removeSelectedText();
    foundPos.insertFragment(replacement);
    foundPos.endEditBlock();
    }

  return foundPos;
  }

void TDocumentTransform::removeContainingLine(const char* textToFind,
  const QTextCursor& startPos /*= QTextCursor()*/)
  {
  QTextCursor foundPos = find(textToFind, startPos);

  if(foundPos.isNull() == false && foundPos.hasSelection())
    {
    foundPos.beginEditBlock();
    foundPos.movePosition(QTextCursor::MoveOperation::StartOfLine, QTextCursor::MoveMode::MoveAnchor);
    foundPos.movePosition(QTextCursor::MoveOperation::EndOfLine, QTextCursor::MoveMode::KeepAnchor);
    foundPos.removeSelectedText();
    foundPos.deletePreviousChar();
    foundPos.endEditBlock();
    }
  }

QTextCursor TDocumentTransform::find(const char* textToFind, const QTextCursor& startPos)
  {
  /// Use strict matching to avoid mismatch while doing replace.
  QTextDocument::FindFlags findOptions = QTextDocument::FindFlags(
    QTextDocument::FindFlag::FindCaseSensitively|QTextDocument::FindFlag::FindWholeWords);
  QTextCursor foundPos = Doc->find(QString(textToFind), startPos, findOptions);
  return foundPos;
  }

} ///namespace

MailEditorMainWindow::MailEditorMainWindow(ATopLevelWindowsContainer* parent, AddressBookModel& abModel,
  IMailProcessor& mailProcessor, bool editMode) :
  ATopLevelWindow(parent),
  ui(new Ui::MailEditorWindow()),
  ABModel(abModel),
  MailProcessor(mailProcessor),
  FontCombo(nullptr),
  EditMode(editMode)
  {
  ui->setupUi(this);

  /** Disable these toolbars by default. They should be showed up on demand, when given action will
      be trigerred.
  */
  ui->fileAttachementToolBar->hide();
  ui->moneyAttachementToolBar->hide();
  ui->editToolBar->hide();
  ui->adjustToolbar->hide();
  ui->formatToolBar->hide();

  MoneyAttachement = new TMoneyAttachementWidget(ui->moneyAttachementToolBar);
  ui->moneyAttachementToolBar->addWidget(MoneyAttachement);

  FileAttachment = new TFileAttachmentWidget(ui->fileAttachementToolBar, editMode);
  ui->fileAttachementToolBar->addWidget(FileAttachment);

  MailFields = new MailFieldsWidget(*this, *ui->actionSend, abModel, editMode);

  /// Initially only basic mail fields (To: and Subject:) should be visible
  MailFields->showCcControls(false);
  MailFields->showBccControls(false);

  ui->mailFieldsToolBar->addWidget(MailFields);

  connect(MailFields, SIGNAL(subjectChanged(QString)), this, SLOT(onSubjectChanged(QString)));
  connect(MailFields, SIGNAL(recipientListChanged()), this, SLOT(onRecipientListChanged()));
  connect(FileAttachment, SIGNAL(attachmentListChanged()), this, SLOT(onAttachmentListChanged()));

  if(editMode)
    {
    /** Supplement definition of mailFieldSelectorToolbar since Qt Creator doesn't support putting
        into its context dedicated controls (like preconfigured toolbutton).

        Setup local menu for 'actionMailFields' toolButton (used to enable/disable additional mail
        field selection).
    */
    QMenu* mailFieldsMenu = new QMenu(this);
    mailFieldsMenu->addAction(ui->actionFrom);
    mailFieldsMenu->addAction(ui->actionCC);
    mailFieldsMenu->addAction(ui->actionBCC);

    /// Update state of sub-menu commands.
    ui->actionBCC->setChecked(MailFields->isFieldVisible(MailFieldsWidget::BCC_FIELDS));
    ui->actionCC->setChecked(MailFields->isFieldVisible(MailFieldsWidget::CC_FIELD));
    ui->actionFrom->setChecked(MailFields->isFieldVisible(MailFieldsWidget::FROM_FIELD));

    ui->actionMailFields->setMenu(mailFieldsMenu);
    ui->mainToolBar->insertAction(ui->actionShowFormatOptions, ui->actionMailFields);
    }

  setupEditorCommands();

  ui->messageEdit->setFocus();
  fontChanged(ui->messageEdit->font());
  colorChanged(ui->messageEdit->textColor());
  alignmentChanged(ui->messageEdit->alignment());

  QString subject = MailFields->getSubject();
  onSubjectChanged(subject);
  
  /// Clear modified flag
  ui->messageEdit->document()->setModified(false);
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

  toggleReadOnlyMode();
  }

MailEditorMainWindow::~MailEditorMainWindow()
  {
  delete ui;
  }

void MailEditorMainWindow::SetRecipientList(const TRecipientPublicKeys& toList,
  const TRecipientPublicKeys& ccList, const TRecipientPublicKeys& bccList)
  {
  TRecipientPublicKey emptyKey;
  MailFields->SetRecipientList(emptyKey, toList, ccList, bccList);
  /// Ignore MailFields::recipientListChanged signal here
  ui->messageEdit->document()->setModified(false);
  setWindowModified(false);
  }

void MailEditorMainWindow::LoadMessage(Mailbox* mailbox, const TStoredMailMessage& srcMsgHeader,
  const TPhysicalMailMessage& srcMsg, TLoadForm loadForm)
  {
  TPublicKeyIndex allRecipients, toRecipients;
  TRecipientPublicKeys sourceToList, sourceCCList;
  QString newSubject;

  switch(loadForm)
    {
    case TLoadForm::Draft:
      DraftMessage = srcMsgHeader;
      /// Now load source message contents into editor controls.
      loadContents(srcMsgHeader.from_key, srcMsg);
      break;
    case TLoadForm::ReplyAll:
      transformRecipientList(srcMsgHeader.from_key, srcMsg.to_list, srcMsg.cc_list);
      newSubject = transformMailBody(loadForm, srcMsgHeader, srcMsg);
      MailFields->SetSubject(newSubject);
      break;
    case TLoadForm::Reply:
      if(srcMsg.to_list.empty() == false)
        sourceToList.push_back(srcMsg.to_list.front());
      transformRecipientList(srcMsgHeader.from_key, sourceToList, sourceCCList);
      newSubject = transformMailBody(loadForm, srcMsgHeader, srcMsg);
      MailFields->SetSubject(newSubject);
      break;
    case TLoadForm::Forward:
      FileAttachment->LoadAttachedFiles(srcMsg.attachments);
      newSubject = transformMailBody(loadForm, srcMsgHeader, srcMsg);
      MailFields->SetSubject(newSubject);
      break;
    default:
      assert(false);
    }
  
  ui->messageEdit->moveCursor(QTextCursor::MoveOperation::Start, QTextCursor::MoveMode::MoveAnchor);
  onFileAttachementTriggered( FileAttachment->hasAttachment() );
  
  if ( !EditMode && FileAttachment->hasAttachment())
    {
    mailbox->previewImages(ui->messageEdit);
    ui->messageEdit->document()->setModified(false);
    }
  }

void MailEditorMainWindow::closeEvent(QCloseEvent *e)
  {
  if(maybeSave())
  {
    e->accept();
    ATopLevelWindow::closeEvent(e);
  }
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
    return onSave();
  else
  if(ret == QMessageBox::Cancel)
    return false;

  return true;
  }

void MailEditorMainWindow::setupEditorCommands()
  {
  QPixmap pix(ui->editToolBar->iconSize());
  pix.fill(Qt::black);
  ui->actionTextColor->setIcon(pix);

  FontCombo = new QFontComboBox(ui->formatToolBar);
  ui->formatToolBar->addWidget(FontCombo);
  connect(FontCombo, SIGNAL(activated(QString)), this, SLOT(onTextFamilyChanged(QString)));

  FontSize = new QComboBox(ui->formatToolBar);
  FontSize->setObjectName("comboSize");
  ui->formatToolBar->addWidget(FontSize);
  FontSize->setEditable(true);

  QFontDatabase db;
  foreach(int size, db.standardSizes())
    FontSize->addItem(QString::number(size));

  connect(FontSize, SIGNAL(activated(QString)), this, SLOT(onTextSizeChanged(QString)));
  FontSize->setCurrentIndex(FontSize->findText(QString::number(QApplication::font().pointSize())));
  }

bool MailEditorMainWindow::isMsgSizeOK(const TPhysicalMailMessage& srcMsg)
{
  int msg_size = srcMsg.subject.size() + srcMsg.body.size();
  for(int i=0; i<srcMsg.attachments.size(); i++)
    msg_size += srcMsg.attachments[i].body.size();
  if(msg_size > 1024*1024)
  {
    QMessageBox::warning(this, tr("Warning"), tr("Message size limit exceeded.\nMessage with attachments can not be larger than 1MB."));
    return false;
  }
  return true;
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
  FontCombo->setCurrentIndex(FontCombo->findText(QFontInfo(f).family()));
  FontSize->setCurrentIndex(FontSize->findText(QString::number(f.pointSize())));

  ui->actionBold->setChecked(f.bold());
  ui->actionItalic->setChecked(f.italic());
  ui->actionUnderline->setChecked(f.underline());
  }

void MailEditorMainWindow::colorChanged(const QColor& c)
  {
  QPixmap pix(16, 16);
  pix.fill(c);
  ui->actionTextColor->setIcon(pix);
  }

void MailEditorMainWindow::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
  {
  QTextCursor cursor = ui->messageEdit->textCursor();
  if(cursor.hasSelection() == false)
    cursor.select(QTextCursor::WordUnderCursor);
  cursor.mergeCharFormat(format);
  ui->messageEdit->mergeCurrentCharFormat(format);
  }

bool MailEditorMainWindow::prepareMailMessage(TPhysicalMailMessage* storage)
  {
  const bts::addressbook::wallet_identity& senderId = MailFields->GetSenderIdentity();
  storage->subject = MailFields->getSubject().toStdString();

  MailFields->FillRecipientLists(&storage->to_list, &storage->cc_list, &storage->bcc_list);
  storage->body = ui->messageEdit->document()->toHtml().toStdString();

  typedef TFileAttachmentWidget::TFileInfoList TFileInfoList;
  TFileInfoList brokenFileInfos;
  if(FileAttachment->GetAttachedFiles(&storage->attachments, &brokenFileInfos) == false)
    {
    /// Report a message about file attachment failure.
    QString msg(tr("Following files doesn't exist or are not readable. Do you want to continue ?<br/>"));
    for(auto fiIt = brokenFileInfos.cbegin(); fiIt != brokenFileInfos.cend(); ++fiIt)
      {
      const QFileInfo& fi = *fiIt;
      msg += fi.absoluteFilePath() + tr("<br/>");
      }
    
    if(QMessageBox::question(this, tr("File attachment"), msg) == QMessageBox::Button::No)
      return false;
    }

  return true;
  }

void MailEditorMainWindow::loadContents(const TRecipientPublicKey& senderId,
  const TPhysicalMailMessage& srcMsg)
  {
  MailFields->LoadContents(senderId, srcMsg);
  FileAttachment->LoadAttachedFiles(srcMsg.attachments);
  ui->messageEdit->setText(QString(srcMsg.body.c_str()));
  }

void MailEditorMainWindow::transformRecipientList(const TRecipientPublicKey& senderId,
  const TRecipientPublicKeys& sourceToList, const TRecipientPublicKeys& sourceCCList)
  {
  TPublicKeyIndex allRecipients, myIdentities;

  TRecipientPublicKeys toRecipients, ccRecipients, empty;
  toRecipients.reserve(sourceToList.size() + 1);
  ccRecipients.reserve(sourceCCList.size());

  /** First fill allRecipient index with own identities public keys, to avoid sending replied
      message to myself
  */
  for(const auto& id : bts::get_profile()->identities())
    {
    const auto& pk = id.public_key;
    assert(pk.valid());
    TInsertInfo ii = allRecipients.insert(pk);
    assert(ii.second && "Redundant identity public keys");
    }

  myIdentities = allRecipients;

  TRecipientPublicKey newSenderId;

  toRecipients.push_back(senderId);
  for(const auto& recipient : sourceToList)
    {
    TInsertInfo ii = allRecipients.insert(recipient);
    if(ii.second)
      toRecipients.push_back(recipient);

    if(newSenderId.valid() == false && myIdentities.find(recipient) != myIdentities.end())
      newSenderId = recipient;
    }

  for(const auto& recipient : sourceCCList)
    {
    TInsertInfo ii = allRecipients.insert(recipient);
    if(ii.second)
      ccRecipients.push_back(recipient);
    }

  MailFields->SetRecipientList(newSenderId, toRecipients, ccRecipients, empty);
  }

QString 
MailEditorMainWindow::transformMailBody(TLoadForm loadForm, const TStoredMailMessage& msgHeader,
  const TPhysicalMailMessage& srcMsg)
  {
  TDocumentTransform transform;
  return transform.Do(loadForm, msgHeader, srcMsg, ui->messageEdit->document());
  }

void MailEditorMainWindow::toggleReadOnlyMode()
  {
  ui->actionCut->setEnabled(EditMode);
  ui->actionPaste->setEnabled(EditMode);
  ui->messageEdit->setReadOnly(EditMode == false);
  ui->formatToolBar->setEnabled(EditMode);
  ui->adjustToolbar->setEnabled(EditMode);
  }

bool MailEditorMainWindow::onSave()
  {
  ui->messageEdit->document()->setModified(false);
  TPhysicalMailMessage msg;
  if(prepareMailMessage(&msg))
    {
      if(!isMsgSizeOK(msg))
      {
        ui->messageEdit->document()->setModified(true);
        return false;
      }
    const IMailProcessor::TIdentity& senderId = MailFields->GetSenderIdentity();

    auto app = bts::application::instance();
    auto profile = app->get_profile();
    auto idents = profile->identities();

    if(0 == idents.size())
      {
      QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("Keyhotee"),
      tr("Cannot save this draft. No Identity is present.\nPlease create an Identity to save this draft"),
      QMessageBox::Ok);
      return false;
      }
    //DLN we should probably add get_pointer implementation to fc::optional to avoid code like this
    TStoredMailMessage* oldMessage = DraftMessage.valid() ? &(*DraftMessage) : nullptr;
    DraftMessage = MailProcessor.Save(senderId, msg, oldMessage);
    }
  return true;
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

void MailEditorMainWindow::onTextColorTriggerred()
  {
  QColor col = QColorDialog::getColor(ui->messageEdit->textColor(), this);
  
  if(col.isValid() == false)
    return;
  QTextCharFormat fmt;
  fmt.setForeground(col);
  mergeFormatOnWordOrSelection(fmt);
  colorChanged(col);
  }

void MailEditorMainWindow::onTextFamilyChanged(const QString& f)
  {
  QTextCharFormat fmt;
  fmt.setFontFamily(f);
  mergeFormatOnWordOrSelection(fmt);
  }

void MailEditorMainWindow::onTextSizeChanged(const QString &p)
  {
  qreal pointSize = p.toFloat();
  if(p.toFloat() > 0)
    {
    QTextCharFormat fmt;
    fmt.setFontPointSize(pointSize);
    mergeFormatOnWordOrSelection(fmt);
    }
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

void MailEditorMainWindow::onShowFormattingControlsTriggered(bool checked)
  {
  ui->formatToolBar->setVisible(checked);
  ui->adjustToolbar->setVisible(checked);
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
  TPhysicalMailMessage msg;
  if(prepareMailMessage(&msg))
    {
      if(!isMsgSizeOK(msg))
        return;
    const IMailProcessor::TIdentity& senderId = MailFields->GetSenderIdentity();
    MailProcessor.Send(senderId, msg, DraftMessage.valid() ? &(*DraftMessage) : nullptr);
    /// Clear potential modified flag to avoid asking for saving changes.
    ui->messageEdit->document()->setModified(false);
    close();
    }
  }

void MailEditorMainWindow::onSubjectChanged(const QString& subject)
  {
  setWindowTitle(tr("Mail message: %1[*]").arg(subject));
  /// Let's treat subject change also as document modification.
  ui->messageEdit->document()->setModified(true);
  }

void MailEditorMainWindow::onRecipientListChanged()
  {
  /// Let's treat recipient list change also as document modification.
  ui->messageEdit->document()->setModified(true);
  }

void MailEditorMainWindow::onAttachmentListChanged()
  {
  /// Let's treat attachment list change also as document modification.
  ui->messageEdit->document()->setModified(true);
  }
