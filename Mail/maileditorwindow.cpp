#include "maileditorwindow.hpp"

#include "ui_maileditorwindow.h"

#include "fileattachmentwidget.hpp"
#include "mailfieldswidget.hpp"
#include "moneyattachementwidget.hpp"
#include "utils.hpp"

#include <bts/profile.hpp>

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QFontComboBox>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
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
    /// Allows to remove whole line containing given text.
    void removeContainingLine(const char* textToFind, const QTextCursor& startPos = QTextCursor());
    QTextCursor find(const char* textToFind, const QTextCursor& startPos);
    /** Remove nested html header from the message body.
    RepliedMailPattern.html already contains html header
    */
    void removeHtmlHeader(QString &body);
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

  QString senderText(Utils::toString(msgHeader.from_key, Utils::FULL_CONTACT_DETAILS));
  QString sentDate(Utils::toQDateTime(msgHeader.from_sig_time).toString(Qt::DefaultLocaleShortDate));
  QString toList(Utils::makeContactListString(srcMsg.to_list, ';', Utils::FULL_CONTACT_DETAILS));
  QString ccList(Utils::makeContactListString(srcMsg.cc_list, ';', Utils::FULL_CONTACT_DETAILS));

  patternHtml.replace("$$SENDER$$", senderText);
  patternHtml.replace("$$SENT_DATE$$", sentDate);
  patternHtml.replace("$$TO_RECIPIENTS$$", toList);
  
  if (!ccList.isEmpty())
  {
    patternHtml.replace("$$CC_RECIPIENTS$$", ccList);
  }
  patternHtml.replace("$$SUBJECT$$", newSubject);

  QString body = QString(srcMsg.body.c_str());
  /** Even forcing the following conversion:
  QTextDocumentFragment tf(QTextDocumentFragment::fromHtml(QString(srcMsg.body.c_str())));
  and next
  replace("$$SOURCE_BODY$$", tf);
  Html tags doesn't work properly for $$SOURCE_BODY$$" section.
  Therefore first replace section $$...$$ in the patternHtml and next insert
  html document to QTextDocument
  */
  removeHtmlHeader(body);
  patternHtml.replace("$$SOURCE_BODY$$", body);
  doc->setHtml(patternHtml);
  if (ccList.isEmpty())
  {
    removeContainingLine("$$CC_RECIPIENTS$$");
  }

  return newSubject;
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

void TDocumentTransform::removeHtmlHeader(QString &body)
{
  /// Remove nested html header
  int pos = 0;
  /// find start of body section
  pos = body.indexOf("<body", pos);
  pos = body.indexOf(">", pos);
  /// remove <head.. and <body..
  body.remove(0, pos + 1);

  /// Find end of body section.
  /// Set position to end of the document not to search the entire message
  pos = body.size() - 100/*sizeof(<body><head> + reserve)*/;
  pos = body.indexOf("</body>", pos);
  assert(pos != -1);
  if (pos != -1)
  {
    /// remove </body..</html.. section
    body.remove(pos, body.size() - pos);
  }
}

MailEditorMainWindow::MailEditorMainWindow(ATopLevelWindowsContainer* parent, AddressBookModel& abModel,
  IMailProcessor& mailProcessor, bool editMode) :
  ATopLevelWindow(parent),
  ui(new Ui::MailEditorWindow()),
  _parent(parent),
  ABModel(abModel),
  MailProcessor(mailProcessor), 
  FontCombo(nullptr),
  EditMode(editMode)
  {
  ui->setupUi(this);
  ui->remoteContentAlert->initial(this);
  ui->messageEdit->initial(this);
  _msg_type = IMailProcessor::TMsgType::Normal;

  /** Disable these toolbars by default. They should be showed up on demand, when given action will
      be trigerred.
  */
  ui->fileAttachementToolBar->hide();
  ui->moneyAttachementToolBar->hide();
  ui->editToolBar->hide();
  ui->adjustToolbar->hide();
  ui->formatToolBar->hide();
  ui->mailToolBar->hide();

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
  else
    ui->mailToolBar->show();

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
  connect(ui->messageEdit, SIGNAL(attachmentAdded(const QStringList&)), this,
    SLOT(onFileAttachmentAdded(const QStringList&)));

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

  _src_msg = srcMsg;

  ui->messageEdit->setOpenExternalLinks(true);
  ui->messageEdit->setOpenLinks(true);

  switch(loadForm)
    {
    case TLoadForm::Draft:
      DraftMessage = srcMsgHeader;
      /// Now load source message contents into editor controls.
      loadContents(srcMsgHeader.from_key, srcMsg);
      _src_msg_id = srcMsg.src_msg_id;
      if(srcMsgHeader.isTempReply())
        _msg_type = IMailProcessor::TMsgType::Reply;
      else if(srcMsgHeader.isTempForwa())
        _msg_type = IMailProcessor::TMsgType::Forward;
      else
        _msg_type = IMailProcessor::TMsgType::Normal;
      break;
    case TLoadForm::ReplyAll:
      transformRecipientList(srcMsgHeader.from_key, srcMsg.to_list, srcMsg.cc_list);
      newSubject = transformMailBody(loadForm, srcMsgHeader, srcMsg);
      MailFields->SetSubject(newSubject);
      _src_msg_id = srcMsgHeader.digest;
      _msg_type = IMailProcessor::TMsgType::Reply;
      break;
    case TLoadForm::Reply:
      if(srcMsg.to_list.empty() == false)
        sourceToList.push_back(srcMsg.to_list.front());
      transformRecipientList(srcMsgHeader.from_key, sourceToList, sourceCCList);
      newSubject = transformMailBody(loadForm, srcMsgHeader, srcMsg);
      MailFields->SetSubject(newSubject);
      _src_msg_id = srcMsgHeader.digest;
      _msg_type = IMailProcessor::TMsgType::Reply;
      break;
    case TLoadForm::Forward:
      FileAttachment->LoadAttachedFiles(srcMsg.attachments);
      newSubject = transformMailBody(loadForm, srcMsgHeader, srcMsg);
      MailFields->SetSubject(newSubject);
      _src_msg_id = srcMsgHeader.digest;
      _msg_type = IMailProcessor::TMsgType::Forward;
      break;
    default:
      assert(false);
    }
  
  ui->messageEdit->moveCursor(QTextCursor::MoveOperation::Start, QTextCursor::MoveMode::MoveAnchor);
  onFileAttachementTriggered( FileAttachment->hasAttachment() ); 
    }

void MailEditorMainWindow::closeEvent(QCloseEvent *e)
  {
  if(maybeSave())
    {
    MailFields->closeEvent();
    e->accept();
    ATopLevelWindow::closeEvent(e);
    }
  else
    {
    e->ignore();
    }
  }

bool MailEditorMainWindow::maybeSave()
  {
  if(ui->messageEdit->document()->isModified() == false)
    return true;

  QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("Keyhotee"),
    tr("The document has been modified.\nDo you want to save your changes ?"),
    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  
  if(ret == QMessageBox::Save)
    {
    auto idents = bts::get_profile()->identities();

    if(idents.empty())
      {
      QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("Keyhotee"),
        tr("Cannot save this draft. No Identity is present.\nPlease create an Identity to save this draft"),
        QMessageBox::Ok);

      return false;
      }

    return onSave();
    }
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

bool MailEditorMainWindow::checkMsgSize(const TPhysicalMailMessage& srcMsg)
{
  size_t msg_size = srcMsg.subject.size() + srcMsg.body.size();
  for(size_t i=0; i<srcMsg.attachments.size(); ++i)
    msg_size += srcMsg.attachments[i].body.size();

  if(msg_size > 1024*1024)
  {
    QMessageBox::warning(this, tr("Warning"),
      tr("Message size limit exceeded.\nMessage with attachments can not be larger than 1 MB."));
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

  storage->src_msg_id = _src_msg_id;

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

  ui->messageEdit->loadContents(srcMsg.body.c_str(), srcMsg.attachments);
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

  auto profile = bts::application::instance()->get_profile();
  ui->mailToolBar->setDisabled(EditMode || profile->identities().empty());
  }

bool MailEditorMainWindow::onSave()
  {
  TPhysicalMailMessage msg;
  if(prepareMailMessage(&msg))
    {
    if(checkMsgSize(msg) == false)
      return false;

    const IMailProcessor::TIdentity& senderId = MailFields->GetSenderIdentity();

    //DLN we should probably add get_pointer implementation to fc::optional to avoid code like this
    TStoredMailMessage* oldMessage = DraftMessage.valid() ? &(*DraftMessage) : nullptr;
    DraftMessage = MailProcessor.Save(senderId, msg, _msg_type, oldMessage);
    }

  ui->messageEdit->document()->setModified(false);

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
    if(checkMsgSize(msg) == false)
      return;

    const IMailProcessor::TIdentity& senderId = MailFields->GetSenderIdentity();
    MailProcessor.Send(senderId, msg, _msg_type, DraftMessage.valid() ? &(*DraftMessage) : nullptr);
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

void MailEditorMainWindow::onFileAttachmentAdded(const QStringList& files)
{
  if (files.isEmpty() == false)
  {
    onFileAttachementTriggered( true );
    FileAttachment->addFiles( files );
  }
}

void MailEditorMainWindow::addContactCard (const Contact& contact)
{
  FileAttachment->addContactCard(contact);
  ui->fileAttachementToolBar->setVisible(true);
}

void MailEditorMainWindow::onMailReplyTriggered()
{
  MailEditorMainWindow* mailEditor = new MailEditorMainWindow(_parent, ABModel, MailProcessor, true);
  mailEditor->LoadMessage(nullptr, *DraftMessage, _src_msg, Reply);
  mailEditor->show();
}

void MailEditorMainWindow::onMailReplyAllTriggered()
{
  MailEditorMainWindow* mailEditor = new MailEditorMainWindow(_parent, ABModel, MailProcessor, true);
  mailEditor->LoadMessage(nullptr, *DraftMessage, _src_msg, ReplyAll);
  mailEditor->show();
}

void MailEditorMainWindow::onMailForwardTriggered()
{
  MailEditorMainWindow* mailEditor = new MailEditorMainWindow(_parent, ABModel, MailProcessor, true);
  mailEditor->LoadMessage(nullptr, *DraftMessage, _src_msg, Forward);
  mailEditor->show();
}

void MailEditorMainWindow::onBlockedImage()
{
  /// Show alert if any remote image is blocked
  ui->remoteContentAlert->show();
}

void MailEditorMainWindow::onLoadBlockedImages()
{
  /// Hide alert about blocking images
  ui->remoteContentAlert->hide();

  /// Clear modified flag
  ui->messageEdit->loadBlockedImages();
}