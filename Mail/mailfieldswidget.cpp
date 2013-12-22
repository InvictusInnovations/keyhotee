#include "mailfieldswidget.hpp"

#include "AddressBook/AddressBookModel.hpp"

#include "ui_mailfieldswidget.h"

#include <bts/application.hpp>
#include <bts/profile.hpp>
#include <bts/address.hpp>

/// QT headers:
#include <QAction>
#include <QMenu>

MailFieldsWidget::MailFieldsWidget(QWidget& parent, QAction& actionSend, AddressBookModel& abModel,
  bool editMode) :
  QWidget(&parent),
  ui(new Ui::MailFieldsWidget),
  ActionSend(actionSend),
  VisibleFields(0),
  EditMode(editMode)
  {
  ui->setupUi(this);

  QCompleter* completer = abModel.getContactCompleter();

  bool readOnly = editMode == false;

  ContactListEdit* recipientEdits[4];
  recipientEdits[0] = ui->toEdit;
  recipientEdits[1] = ui->ccEdit;
  recipientEdits[2] = ui->bccEdit;
  recipientEdits[3] = ui->fromEdit;

  for(unsigned int i = 0; i < sizeof(recipientEdits)/sizeof(ContactListEdit*); ++i)
    {
    ContactListEdit* edit = recipientEdits[i];
    edit->setCompleter(completer);
    edit->setReadOnly(readOnly);

    connect(edit->document(), SIGNAL(contentsChanged()), this, SLOT(onRecipientListChanged()));
    }

  ui->fromEdit->setCompleter(completer);
  if(editMode)
    fillSenderIdentities();

  validateSendButtonState();

  ui->fromButton->setEnabled(editMode);
  ui->toButton->setEnabled(editMode);
  ui->ccButton->setEnabled(editMode);
  ui->bccButton->setEnabled(editMode);

  ui->subjectEdit->setReadOnly(readOnly);
  
  ui->sendButton->setEnabled(editMode);
  ui->sendButton->setVisible(editMode);
  }

MailFieldsWidget::~MailFieldsWidget()
  {
  delete ui;
  }

void MailFieldsWidget::SetRecipientList(const TRecipientPublicKeys& toList,
  const TRecipientPublicKeys& ccList, const TRecipientPublicKeys& bccList)
  {
  ui->toEdit->SetCollectedContacts(toList);
  ui->ccEdit->SetCollectedContacts(ccList);
  ui->bccEdit->SetCollectedContacts(bccList);

  if(ccList.empty() == false)
    showCcControls(true);

  if(bccList.empty() == false)
    showBccControls(true);
  }

void MailFieldsWidget::LoadContents(const TRecipientPublicKey& senderPK,
  const TPhysicalMailMessage& srcMsg)
  {
  ui->fromEdit->clear();

  if(senderPK.valid())
    {
    selectSenderIdentity(senderPK);
    showFromControls(true);
    }

  SetRecipientList(srcMsg.to_list, srcMsg.cc_list, srcMsg.bcc_list);

  ui->subjectEdit->setText(QString(srcMsg.subject.c_str()));
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
  return ui->subjectEdit->text();
  }

void MailFieldsWidget::FillRecipientLists(TRecipientPublicKeys* toList, TRecipientPublicKeys* ccList,
  TRecipientPublicKeys* bccList) const
  {
  ui->toEdit->GetCollectedContacts(toList);
  ui->ccEdit->GetCollectedContacts(ccList);
  ui->bccEdit->GetCollectedContacts(bccList);
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
  bool anyRecipient = ui->toEdit->document()->isEmpty() == false;
  anyRecipient = anyRecipient || ui->ccEdit->document()->isEmpty() == false;
  anyRecipient = anyRecipient || ui->bccEdit->document()->isEmpty() == false;

  /// Make the send button enabled only if there is any recipient
  ui->sendButton->setEnabled(anyRecipient);
  }

void MailFieldsWidget::fillSenderIdentities()
  {
  QMenu* menu = new QMenu(this);
  ui->fromButton->setMenu(menu);

  QAction* first = nullptr;

  auto profile = bts::application::instance()->get_profile();
  std::vector<bts::addressbook::wallet_identity> identities = profile->identities();

  for(const auto& identity : identities)
    {
    std::string entry = identity.getDisplayName();
    auto ipk = identity.public_key;
    assert(ipk.valid());

    QAction* action = menu->addAction(tr(entry.c_str()));
    action->setCheckable(true);
    Action2Identity.insert(TAction2IdentityIndex::value_type(action, identity));
    
    if(first == nullptr)
      first = action;
    }

  connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(onFromBtnTriggered(QAction*)));

  onFromBtnTriggered(first);
  menu->setActiveAction(first);
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
  ui->fromEdit->SetCollectedContacts(sender);
  }

inline
bool MailFieldsWidget::isFieldVisible(TVisibleFields field) const
  {
  return (VisibleFields & field) != 0;
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
  assert(foundPos != Action2Identity.end());

  SenderIdentity = foundPos->second;
  setChosenSender(SenderIdentity.public_key);
  }

