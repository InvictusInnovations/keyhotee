#include "ContactListEdit.hpp"

#include "KeyhoteeMainWindow.hpp"

#include "AddressBook/AddressBookModel.hpp"
#include "AddressBook/Contact.hpp"
#include "AddressBook/ContactCompleterModel.hpp"

#include "public_key_address.hpp"
#include "utils.hpp"

#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QAbstractTextDocumentLayout>
#include <QCompleter>
#include <QKeyEvent>
#include <QMimeData>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include <QToolTip>
#include <QMenu>

#include <fc/log/logger.hpp>

class ContactListEdit::TAutoSkipCompletion
  {
  public:
    TAutoSkipCompletion(ContactListEdit* edit) : _this(edit)
      {
      _this->_skipCompletion = true;
      }

    ~TAutoSkipCompletion()
      {
      _this->_skipCompletion = false;
      }

  private:
    ContactListEdit* _this;
  };

ContactListEdit::ContactListEdit(QWidget* parent)
  : QTextEdit(parent),
  _addressBookModel(nullptr),
  _completerModel(nullptr),
  _skipCompletion(false)
  {
  _completer = nullptr;

  connect(this, &QTextEdit::textChanged, this, &ContactListEdit::onTextChanged);

  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
  fitHeightToDocument();

  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  setTabChangesFocus(true);
  }

ContactListEdit::~ContactListEdit()
  {
  delete _completer;
  _completer = nullptr;
  delete _completerModel;
  _completerModel = nullptr;
  }

void ContactListEdit::insertCompletion(const QModelIndex& completionIndex)
  {
  if(completionIndex.isValid() == false)
    return;

  TAutoSkipCompletion asc(this);

  QString completion = completionIndex.data(Qt::DisplayRole).toString();

  ilog("insertCompletion got completion text: ${c}", ("c", completion.toStdString()));

  /** \warning QCompleter passes here its index relative to ITS model which is a filtering one model.
      Then this index depends on number of items displayed in completer window. Then index is smaller
      when items are matched more strictly (0 in case just 1 row is matched).
      To get actual index, it must be remapped but there is a lacking support in QCompleter API
      to remap such index directly. Indead of cast is needed.
      \see http://qt-project.org/forums/viewthread/26959 for details
  */
  const QAbstractItemModel* completionModel = completionIndex.model();
  const QAbstractProxyModel* proxyModel = dynamic_cast<const QAbstractProxyModel*>(completionModel);
  assert(proxyModel != nullptr);
  QModelIndex sourceIndex = proxyModel->mapToSource(completionIndex);

  const Contact& contact = _completerModel->getContact(sourceIndex);

  assert(completion.toStdString() == contact.get_display_name());

  ilog("insertCompletion chosen contact: ${c}", ("c", contact.get_display_name()));

  deleteEnteredText();
  addContactEntry(completion, contact, false);
  }

void ContactListEdit::onCompleterRequest()
  {
  setFocus();
  initCompleter(QString());
  showCompleter();
  if(_completerModel->rowCount() == 0)
    {
    QRect pos = cursorRect();
    QToolTip::showText(mapToGlobal(pos.topLeft()), tr("There is no contact defined"));
    }
  }

//! [5]
QString ContactListEdit::textUnderCursor(QTextCursor* filledCursor /*= nullptr*/) const
  {
  QTextCursor text_cursor = textCursor();

  int position_of_cursor = text_cursor.position();

  QString text = toPlainText();

  if (text.isEmpty() == false)
  {
    do
      {
      text_cursor.movePosition ( QTextCursor::Left, QTextCursor::MoveAnchor);
      int pos_cursor = text_cursor.position();
      QChar prev_char = text.at(pos_cursor);
      if (prev_char==QChar(QChar::ObjectReplacementCharacter))
        {
        text_cursor.movePosition ( QTextCursor::Right, QTextCursor::MoveAnchor);
        break;
        }
      }
    while(text_cursor.position() != 0);
  }

  text_cursor.setPosition(position_of_cursor, QTextCursor::KeepAnchor);

  if(filledCursor != nullptr)
    *filledCursor = text_cursor;

  QString completionPrefix = text_cursor.selectedText();

  completionPrefix = Utils::lTrim(completionPrefix);

  return completionPrefix;
  }

void ContactListEdit::deleteEnteredText()
  {
  /// Since previously entered prefix has been accepted on completer list, it should be removed
  if(_activeCursor.isNull() == false)
    {
    _activeCursor.removeSelectedText();
    _activeCursor = QTextCursor();
    }
  }

void ContactListEdit::addContactEntry(const QString& contactText, const bts::addressbook::contact& c,
  bool rawPublicKey, const QString* entryTooltip /*= nullptr*/)
  {
  QFont        default_font;
  default_font.setPointSize( default_font.pointSize() - 1 );
  default_font.setBold(rawPublicKey);

  QFontMetrics font_metrics(default_font);
  QRect        bounding = font_metrics.boundingRect(contactText);
  int          completion_width = font_metrics.width(contactText);
  int          completion_height = bounding.height();

  completion_width += 20;

  QImage   completion_image(completion_width, completion_height + 4, QImage::Format_ARGB32);
  completion_image.fill(QColor(0, 0, 0, 0) );
  QPainter painter;
  painter.begin(&completion_image);
  painter.setFont(default_font);
  painter.setRenderHint(QPainter::Antialiasing);

  QBrush brush(Qt::SolidPattern);
  brush.setColor( QColor( 205, 220, 241 ) );
  QPen  pen, textColorPen;

  bool isKeyhoteeFounder = rawPublicKey == false && Contact::isKeyhoteeFounder(c);

  if (isKeyhoteeFounder)
    {
    QLinearGradient grad(QPointF(0, 0), QPointF(0, 1));
    grad.setCoordinateMode(QGradient::ObjectBoundingMode);
    grad.setColorAt(0, QColor(35, 40, 3));
    grad.setColorAt(0.102273, QColor(136, 106, 22));
    grad.setColorAt(0.225, QColor(166, 140, 41));
    grad.setColorAt(0.285, QColor(204, 181, 74));
    grad.setColorAt(0.345, QColor(235, 219, 102));
    grad.setColorAt(0.415, QColor(245, 236, 112));
    grad.setColorAt(0.52, QColor(209, 190, 76));
    grad.setColorAt(0.57, QColor(187, 156, 51));
    grad.setColorAt(0.635, QColor(168, 142, 42));
    grad.setColorAt(0.695, QColor(202, 174, 68));
    grad.setColorAt(0.75, QColor(218, 202, 86));
    grad.setColorAt(0.815, QColor(208, 187, 73));
    grad.setColorAt(0.88, QColor(187, 156, 51));
    grad.setColorAt(0.935, QColor(137, 108, 26));
    grad.setColorAt(1, QColor(35, 40, 3));

    brush = QBrush(grad);
    pen.setColor( QColor( 103, 51, 1 ) );
    }
  else
    {
    if(rawPublicKey)
      {
      brush.setColor(QColor(Qt::darkGreen));
      textColorPen.setColor(QColor(Qt::white));
      }
    else
      {
      brush.setColor( QColor( 205, 220, 241 ) );
      pen.setColor( QColor( 105,110,180 ) );
      }
    }

  painter.setBrush(brush);
  painter.setPen(pen);
  painter.drawRoundedRect(0, 0, completion_width - 1, completion_image.height() - 1, 8, 8,
    Qt::AbsoluteSize);
  painter.setPen(textColorPen);
  painter.drawText(QPoint(10, completion_height - 2), contactText);

  QTextDocument* doc = document();
  doc->addResource(QTextDocument::ImageResource, QUrl(contactText), completion_image);
  QTextImageFormat format;
  format.setName(contactText);

  encodePublicKey(c.public_key, &format);

  if(entryTooltip != nullptr)
    format.setToolTip(*entryTooltip);

  QTextCursor txtCursor = textCursor();
  txtCursor.insertImage(format);
  setTextCursor(txtCursor);
  }

void ContactListEdit::encodePublicKey(const IMailProcessor::TRecipientPublicKey& key,
  QTextImageFormat* storage) const
  {
  assert(key.valid());

  auto pkData = key.serialize();

  QByteArray pkArray(pkData.size(), Qt::Initialization::Uninitialized);
  memcpy(pkArray.data(), pkData.begin(), pkData.size());

  storage->setProperty(QTextImageFormat::UserProperty, QVariant(pkArray));
  }

void ContactListEdit::decodePublicKey(const QTextImageFormat& storage,
  IMailProcessor::TRecipientPublicKey* key) const
  {
  assert(storage.hasProperty(QTextImageFormat::UserProperty));

  QVariant v = storage.property(QTextImageFormat::UserProperty);
  QByteArray pkArray = v.toByteArray();

  fc::ecc::public_key_data s;

  assert(pkArray.size() == s.size());

  memcpy(s.begin(), pkArray.data(), s.size());

  *key = IMailProcessor::TRecipientPublicKey(s);
  assert(key->valid());
  }

void ContactListEdit::focusInEvent(QFocusEvent* focus_event)
  {
  if (_completer)
    _completer->setWidget(this);
  QTextEdit::focusInEvent(focus_event);
  }

bool ContactListEdit::focusNextPrevChild(bool next)
  {
  if (_completer && _completer->popup()->isVisible())
    return false;
  return QTextEdit::focusNextPrevChild(next);
  }

void ContactListEdit::keyPressEvent(QKeyEvent* key_event)
  {
  if (_completer && _completer->popup()->isVisible())
    {
    // The following keys are forwarded by the completer to the widget
    switch (key_event->key())
      {
      case Qt::Key_Enter:
      case Qt::Key_Return:
      case Qt::Key_Escape:
      case Qt::Key_Tab:
      case Qt::Key_Backtab:
        key_event->ignore();
        return;       // let the completer do default behavior
      default:
        break;
      }
    }

  bool isShortcut = ((key_event->modifiers() & Qt::ControlModifier) && key_event->key() == Qt::Key_E);   // CTRL+E
  if (!_completer || !isShortcut)   // do not process the shortcut when we have a completer
    QTextEdit::keyPressEvent(key_event);
  //! [7]

  //! [8]
  const bool ctrlOrShift = key_event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
  if (!_completer || (ctrlOrShift && key_event->text().isEmpty()))
    return;

  bool    hasModifier = (key_event->modifiers() != Qt::NoModifier) && !ctrlOrShift;
  QString completionPrefix = textUnderCursor();

  if (!isShortcut && (hasModifier || key_event->text().isEmpty() || completionPrefix.isEmpty() ))
    {
    _completer->popup()->hide();
    return;
    }
  }

void ContactListEdit::setAddressBookModel(AddressBookModel& abModel)
  {
  assert(_addressBookModel == nullptr);
  _addressBookModel = &abModel;
  _completerModel = new TContactCompletionModel(&abModel);

  //create completer from completion model
  _completer = new QCompleter(_completerModel, this);

  _completer->setWidget(this);
  _completer->setCompletionMode(QCompleter::PopupCompletion);
  _completer->setCaseSensitivity(Qt::CaseInsensitive);
  _completer->setFilterMode(Qt::MatchFlag::MatchContains);
  _completer->setWrapAround(true);

   connect(_completer, SIGNAL(activated(const QModelIndex&)),
           this, SLOT(insertCompletion(const QModelIndex&)));
  }

void ContactListEdit::initCompleter(const QString& completionPrefix)
{
  if (completionPrefix != _completer->completionPrefix())
  {
    _completer->setCompletionPrefix(completionPrefix);
    _completer->popup()->setCurrentIndex(_completer->completionModel()->index(0, 0));
  }
}

void ContactListEdit::showCompleter()
  {
  QRect cr = cursorRect();
  cr.setWidth(_completer->popup()->sizeHintForColumn(0)
              + _completer->popup()->verticalScrollBar()->sizeHint().width());
  _completer->complete(cr);   // popup it up!
  }

void ContactListEdit::SetCollectedContacts(const IMailProcessor::TRecipientPublicKeys& storage)
{
  TAutoSkipCompletion asc(this);

  for(const auto& recipient : storage)
  {
    assert(recipient.valid());
    bool isKnownContact = false;
    bts::addressbook::contact matchingContact;
    QString entryText(Utils::toString(recipient, Utils::TContactTextFormatting::FULL_CONTACT_DETAILS,
      &matchingContact, &isKnownContact));
    addContactEntry(entryText, matchingContact, isKnownContact == false);
  }
}

void ContactListEdit::GetCollectedContacts(IMailProcessor::TRecipientPublicKeys* storage) const
  {
  assert(storage != nullptr);
  storage->reserve(10);

  QTextBlock block = document()->begin();
  while(block.isValid())
    {
    for(QTextBlock::iterator i = block.begin(); i != block.end(); ++i)
      {
      QTextCharFormat format = i.fragment().charFormat();
      if(format.isImageFormat())
        {
        QTextImageFormat imgF = format.toImageFormat();
        IMailProcessor::TRecipientPublicKey pk;
        decodePublicKey(imgF, &pk);
        assert(pk.valid());
        storage->push_back(pk);
        }
      }

    block = block.next();
    }
  }
//! [8]

QSize ContactListEdit::sizeHint() const
  {
  QSize sizehint = QTextEdit::sizeHint();
  //     sizehint.setHeight(_fitted_height);
  return sizehint;
  }

void ContactListEdit::onTextChanged()
  {
  /** Remember last changed text range in _activeCursor member. Will be used to replace entered text
      whith contact entry.
  */
  QString text = textUnderCursor(&_activeCursor);

  if(_skipCompletion == false && _completer != nullptr && isReadOnly() == false &&
     text.isEmpty() == false)
    {
    TAutoSkipCompletion asc(this);
    
    std::string textKey = text.toStdString();

    bts::addressbook::wallet_contact matchedContact;

    bool contactFound = false;
    /// Initializing _completer
    initCompleter(text);
    /** \warning Before checking _completer->completionCount() 
        initCompleter method should be called.
        Find the contact only when one completion exist,
        (so contact will not be replaced with image control holding contact info)
        because autocompletion didn't give suggestion if there were 2 contacts of same id
    */
    if (_completer->completionCount() == 1)
    {
      try
      {
        auto aBook = bts::get_profile()->get_addressbook();

        auto contact = aBook->get_contact_by_display_name(textKey);
        contactFound = contact.valid();
        if (contactFound == false)
          contact = aBook->get_contact_by_dac_id(textKey);

        if (contact)
        {
          contactFound = true;
          matchedContact = *contact;
        }
      }
      catch (const fc::exception& e)
      {
        wlog("${e}", ("e", e.to_detail_string()));
      }
    }

    bool semanticallyValidKey = false;
    if (contactFound)
      {
      /// Delete previously entered text - will be replaced with image control holding contact info
      deleteEnteredText();
      /// Update PK-like text with displayed form
      text = QString::fromStdString(matchedContact.get_display_name());
      /// If text already points to some found contact, just add it
      addContactEntry(text, matchedContact, false);
      }
    else
    if(public_key_address::is_valid(textKey, &semanticallyValidKey) && semanticallyValidKey)
      {
      /// Delete previously entered text - will be replaced with image control holding contact info
      deleteEnteredText();

      public_key_address converter(textKey);
      fc::ecc::public_key parsedKey = converter.key;
      
      if(Utils::matchContact(parsedKey, &matchedContact))
        {
        QString displayText = _completerModel->getDisplayText(Contact(matchedContact));
        addContactEntry(displayText, matchedContact, false);

        QString txt = tr("Known public key has been replaced with matching contact...");
        /// Don't pass here parent widget to avoid switching active window when tooltip appears
        QToolTip::showText(QCursor::pos(), txt, nullptr, QRect(), 3000);
        }
      else
        {
        QString tooltip = tr("Valid, but <b>unknown</b> public key, cannot be matched to any contact "
          "present in address book...");
        addContactEntry(text, matchedContact, true, &tooltip);
        }
      }
    else
      {
      QTextCharFormat fmt;
      fmt.setFontWeight(QFont::Bold);
      fmt.setForeground(QColor(Qt::red));
      fmt.setToolTip(tr("Unknown contact - can be <b>ignored</b> while sending message"));
      _activeCursor.mergeCharFormat(fmt);

      showCompleter();
      }
    }

  fitHeightToDocument();
  }

void ContactListEdit::fitHeightToDocument()
  {
  document()->setTextWidth(width());
  QSize document_size(document()->size().toSize());
  //    _fitted_height = document_size.height();

  setMaximumHeight(document_size.height());
  updateGeometry();
  }

void ContactListEdit::resizeEvent(QResizeEvent* resize_event)
  {
  fitHeightToDocument();
  QTextEdit::resizeEvent(resize_event);
  }

QMimeData *ContactListEdit::createMimeDataFromSelection() const
  {
  //QTextEdit::createMimeDataFromSelection();
  QString       textMime;
  QMimeData     *mimeData = new QMimeData ();
  QTextCursor   cursor = textCursor();
  int           posStart = cursor.selectionStart();
  int           posEnd = cursor.selectionEnd();  
  QTextBlock    block = this->document()->findBlock(posStart);
  QTextBlock    endBlock = this->document()->findBlock(posEnd);
  endBlock = endBlock.next();

  while (block.isValid() && block != endBlock)
    {
    for (QTextBlock::iterator i = block.begin(); !i.atEnd(); ++i)
      {
      int position = i.fragment().position();
      //qDebug() << i.fragment().position();
      if (position >= posEnd) break;
      if (position >= posStart)
        {
        QTextCharFormat format = i.fragment().charFormat();
        bool isImage = format.isImageFormat();
        if (isImage)
          {            
          //qDebug() << format.toImageFormat().name();
          textMime += format.toImageFormat().name();
          }
        else
          textMime += i.fragment().text();
        }
      }
      block = block.next();
    }

  mimeData->setText(textMime);
  return mimeData;
  }

void ContactListEdit::contextMenuEvent ( QContextMenuEvent * event )
{
  QMenu *menu = createStandardContextMenu();

  QAction* sep = new QAction(this);
  sep->setSeparator(true);
  menu->addAction(sep);

  QAction*  action_add_contact = new QAction(tr("Add Contact"), this);
  menu->addAction(action_add_contact);
  connect(action_add_contact, &QAction::triggered, this, &ContactListEdit::onActiveAddContact);
  action_add_contact->setDisabled(true);

  QAction*  action_find_contact = new QAction(tr("Find Contact"), this);
  menu->addAction(action_find_contact);
  connect(action_find_contact, &QAction::triggered, this, &ContactListEdit::onActiveFindContact);
  action_find_contact->setDisabled(true);

  if(textCursor().selection().isEmpty())
  {
    bool isExistingContact = false;
    if(validateClickPosition(event->pos(), &isExistingContact, &_clicked_contact))
    {
      if(isExistingContact)
        action_find_contact->setEnabled(true);
      else
        action_add_contact->setEnabled(true);
    }
  }
  
  menu->exec(event->globalPos());
  delete menu;
}

bool ContactListEdit::validateClickPosition(const QPoint& position, bool* isExistingContact,
  bts::addressbook::wallet_contact* foundContact) const
{
  assert(isExistingContact != nullptr);
  assert(foundContact != nullptr);

  *isExistingContact = false;

  QPoint shiftedPos(position);
  shiftedPos += QPoint(2, 0);

  int current_position = document()->documentLayout()->hitTest(shiftedPos, Qt::ExactHit);

  if(current_position > -1)
  {
    QTextBlock current_block = document()->begin();
    while(current_block.isValid())
    {
      for(QTextBlock::iterator it = current_block.begin(); !(it.atEnd()); ++it)
      {
        QTextFragment current_fragment = it.fragment();
        if(current_fragment.isValid() && current_fragment.contains(current_position))
        {
          QTextFormat current_format = current_fragment.charFormat();
          if(current_format.isImageFormat())
          {
            QTextImageFormat imageFormat = current_format.toImageFormat();

            IMailProcessor::TRecipientPublicKey pk;
            decodePublicKey(imageFormat, &pk);
            *isExistingContact = Utils::matchContact(pk, foundContact);

            return true;
          }
        }
      }
      current_block = current_block.next();
    }
  }

  return false;
}

void ContactListEdit::onActiveAddContact()
{
  getKeyhoteeWindow()->addToContacts(_clicked_contact);
  getKeyhoteeWindow()->activateMainWindow();
}

void ContactListEdit::onActiveFindContact()
{
  getKeyhoteeWindow()->openContactGui(_clicked_contact.wallet_index);
  getKeyhoteeWindow()->activateMainWindow();
}
