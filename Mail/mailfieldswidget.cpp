#include "mailfieldswidget.hpp"
#include "ui_mailfieldswidget.h"

#include "AddressBook/AddressBookModel.hpp"
#include "Identity/IdentityObservable.hpp"

#include <bts/application.hpp>
#include <bts/profile.hpp>
#include <bts/address.hpp>

/// QT headers:
#include <QAction>
#include <QMenu>

const unsigned int SUBJECT_LEN_LIMIT = 1024;

MailFieldsWidget::MailFieldsWidget(QWidget& parent, QAction& actionSend, AddressBookModel& abModel,
  bool editMode) :
  QWidget(&parent),
  ui(new Ui::MailFieldsWidget),
  ActionSend(actionSend),
  VisibleFields(0),
  EditMode(editMode)
  {
  ui->setupUi(this);

  /// Initially hide all controls to show what we will need
  showFromControls(false);
  showBccControls(false);
  showCcControls(false);
  
  bool readOnly = editMode == false;

  ContactListEdit* recipientEdits[4];
  recipientEdits[0] = ui->toEdit;
  recipientEdits[1] = ui->ccEdit;
  recipientEdits[2] = ui->bccEdit;
  recipientEdits[3] = ui->fromEdit;

  for(unsigned int i = 0; i < sizeof(recipientEdits)/sizeof(ContactListEdit*); ++i)
    {
    ContactListEdit* edit = recipientEdits[i];
    edit->setAddressBookModel(abModel);
    edit->setReadOnly(readOnly);

    connect(edit->document(), SIGNAL(contentsChanged()), this, SLOT(onRecipientListChanged()));
    }

  /// This edit must be always read only - it is automatically filled by 'from' tool button selection.
  ui->fromEdit->setReadOnly(true);
  if(editMode)
    fillSenderIdentities();

  ui->sendButton->setVisible(editMode);

  validateSendButtonState();

  ui->fromButton->setEnabled(editMode);
  ui->toButton->setEnabled(editMode);
  ui->ccButton->setEnabled(editMode);
  ui->bccButton->setEnabled(editMode);

  ui->subjectEdit->setReadOnly(readOnly);
  }

MailFieldsWidget::~MailFieldsWidget()
  {  
  delete ui;
  }

void MailFieldsWidget::SetRecipientList(const TRecipientPublicKey& senderPK,
  const TRecipientPublicKeys& toList, const TRecipientPublicKeys& ccList,
  const TRecipientPublicKeys& bccList)
  {
  if(senderPK.valid())
    {
    selectSenderIdentity(senderPK);
    showFromControls(true);
    }

  ui->toEdit->SetCollectedContacts(toList);
  ui->ccEdit->SetCollectedContacts(ccList);
  ui->bccEdit->SetCollectedContacts(bccList);

  if(ccList.empty() == false)
    showCcControls(true);

  if(bccList.empty() == false)
    showBccControls(true);
  }

void MailFieldsWidget::SetSubject(const std::string& subject)
  {
  SetSubject(QString(subject.c_str()));
  }

void MailFieldsWidget::SetSubject(const QString& subject)
  {
  ui->subjectEdit->setText(trimSubject(subject));
  }

void MailFieldsWidget::LoadContents(const TRecipientPublicKey& senderPK,
  const TPhysicalMailMessage& srcMsg)
  {
  SetRecipientList(senderPK, srcMsg.to_list, srcMsg.cc_list, srcMsg.bcc_list);

  SetSubject(srcMsg.subject);
  }

void MailFieldsWidget::showFromControls(bool show)
  {
  showChildLayout(ui->fromLayout, show, 0, TVisibleFields::FROM_FIELD);
  }

void MailFieldsWidget::showCcControls(bool show)
  {
  /** Check if field placed at previous position is displayed, if not decrease position to avoid
      field order violation.
  */
  int preferredPosition = 2;
  if(show && isFieldVisible(TVisibleFields::FROM_FIELD) == false)
    --preferredPosition;

  showChildLayout(ui->ccLayout, show, preferredPosition, TVisibleFields::CC_FIELD);
  }

void MailFieldsWidget::showBccControls(bool show)
  {
  /** Check if field placed at previous position is displayed, if not decrease position to avoid
      field order violation.
  */
  int preferredPosition = 3;
  if(show)
    {
    if(isFieldVisible(TVisibleFields::FROM_FIELD) == false)
      --preferredPosition;

    if(isFieldVisible(TVisibleFields::CC_FIELD) == false)
      --preferredPosition;
    }

  showChildLayout(ui->bccLayout, show, preferredPosition, TVisibleFields::BCC_FIELDS);
  }

QString MailFieldsWidget::getSubject() const
  {
  QString sourceText = ui->subjectEdit->text();
  return trimSubject(sourceText);
  }

void MailFieldsWidget::FillRecipientLists(TRecipientPublicKeys* toList, TRecipientPublicKeys* ccList,
  TRecipientPublicKeys* bccList) const
  {
  ui->toEdit->GetCollectedContacts(toList);
  ui->ccEdit->GetCollectedContacts(ccList);
  ui->bccEdit->GetCollectedContacts(bccList);
  }

inline
QString MailFieldsWidget::trimSubject(const QString& sourceText) const
  {
  if(sourceText.length() > SUBJECT_LEN_LIMIT)
    return sourceText.left(SUBJECT_LEN_LIMIT);

  return sourceText;
  }

void MailFieldsWidget::showChildLayout(QLayout* layout, bool show, int preferredPosition,
  unsigned int fieldFlag)
  {
  if(show)
    VisibleFields |= fieldFlag;
  else
    VisibleFields &= ~fieldFlag;

  ui->mailFieldsLayout->setEnabled(false);
  if(show)
    {
    int maxCnt = ui->mailFieldsLayout->count();
    if(preferredPosition >= maxCnt)
      preferredPosition = maxCnt;

    ui->mailFieldsLayout->insertLayout(preferredPosition, layout);
    }
  else
    {
    int itemIdx = 0;
    for(itemIdx = 0; itemIdx < ui->mailFieldsLayout->count(); ++itemIdx)
      {
      if(ui->mailFieldsLayout->itemAt(itemIdx) == layout)
        break;
      }

    ui->mailFieldsLayout->takeAt(itemIdx);
    }

  showLayoutWidgets(layout, show);

  ui->mailFieldsLayout->setEnabled(true);
  ui->mailFieldsLayout->invalidate();
  }

void MailFieldsWidget::showLayoutWidgets(QLayout* layout, bool show)
  {
  for(int i = 0; i < layout->count(); ++i)
    {
    QLayoutItem* li = layout->itemAt(i);
    QWidget* w = li->widget();
    w->setVisible(show);
    }
  }

void MailFieldsWidget::validateSendButtonState()
  {
  bool anySender = Action2Identity.empty() == false;
  auto profile = bts::application::instance()->get_profile();
  anySender = anySender && profile->identities().size();

  IMailProcessor::TRecipientPublicKeys recipients;
  ui->toEdit->GetCollectedContacts(&recipients);
  ui->ccEdit->GetCollectedContacts(&recipients);
  ui->bccEdit->GetCollectedContacts(&recipients);

  bool anyRecipient = recipients.empty() == false;
  /// Make the send button enabled only if there is any recipient
  ui->sendButton->setEnabled(anyRecipient && anySender);
  }

void MailFieldsWidget::selectSenderIdentity(const TRecipientPublicKey& senderPK)
  {
  assert(senderPK.valid());

  for(const auto& it : Action2Identity)
    {
    const IMailProcessor::TIdentity& id = it.second;
    assert(id.public_key.valid());

    if(id.public_key == senderPK)
      {
      onFromBtnTriggered(it.first);
      return;
      }
    }

  assert(EditMode == false && "When editing an email only one of profile identities could be used"
    "as sender.");

  setChosenSender(senderPK);
  }

void MailFieldsWidget::setChosenSender(const TRecipientPublicKey& senderPK)
  {
  TRecipientPublicKeys sender;
  sender.push_back(senderPK);
  ui->fromEdit->clear();
  ui->fromEdit->SetCollectedContacts(sender);
  }

void MailFieldsWidget::on_sendButton_clicked()
  {
  ActionSend.trigger();
  }

void MailFieldsWidget::onRecipientListChanged()
  {
  validateSendButtonState();
  emit recipientListChanged();
  }

void MailFieldsWidget::onSubjectChanged(const QString &subject)
  {
  emit subjectChanged(subject);
  }

void MailFieldsWidget::onFromBtnTriggered(QAction* action)
  {
  /// Clear checked state for all identities
  for(const auto& v : Action2Identity)
    v.first->setChecked(false);

  action->setChecked(true);

  TAction2IdentityIndex::const_iterator foundPos = Action2Identity.find(action);
  if(foundPos != Action2Identity.end())
    {
    SenderIdentity = foundPos->second;
    setChosenSender(SenderIdentity.public_key);
    }
  }

void MailFieldsWidget::fillSenderIdentities()
  {
  QMenu* menu = new QMenu(this);
  ui->fromButton->setMenu(menu);

  connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(onFromBtnTriggered(QAction*)));

  /// add identity observer
  IdentityObservable::getInstance().addObserver(this);
  }

void MailFieldsWidget::onIdentitiesChanged(const TIdentities& identities)
{
  QMenu* menu = ui->fromButton->menu();
  assert (menu != nullptr);

  /// Clear and fill again
  menu->clear();
  Action2Identity.clear();

  QAction* initial = nullptr;

  for(const auto& identity : identities)
    {
    std::string entry = identity.get_display_name();
    auto ipk = identity.public_key;
    assert(ipk.valid());

    QAction* action = menu->addAction(entry.c_str());
    action->setCheckable(true);
    Action2Identity.insert(TAction2IdentityIndex::value_type(action, identity));

    if (initial == nullptr)
      initial = action;

    if (SenderIdentity.public_key == identity.public_key)
      {
      /// save current selected identity (don't clear selection)
      initial = action;
      }
    }

  /** \warning nowadays because of broken profile creation process, application can be in inconsistent
      state and have no identity defined.
  */
  if (initial != nullptr)
    {
    onFromBtnTriggered(initial);
    menu->setActiveAction(initial);
    }
  else
    {
    QAction* action = menu->addAction(tr("No identity defined"));
    action->setDisabled(true);
    }

  /// Show from controls when multiple identities are defined.
  bool showFromControl = Action2Identity.size() > 1;
  showFromControls(showFromControl);  

  validateSendButtonState();
}

void MailFieldsWidget::closeEvent()
{
  IdentityObservable::getInstance().deleteObserver(this);
}
