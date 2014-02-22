#include "ContactListEdit.hpp"

#include "utils.hpp"

#include "KeyhoteeMainWindow.hpp"
#include "AddressBook/AddressBookModel.hpp"
#include "utils.hpp"

#include <QAbstractItemView>
#include <QCompleter>
#include <QKeyEvent>
#include <QMimeData>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include <QToolTip>
#include <QMenu>

#include <fc/log/logger.hpp>

ContactListEdit::ContactListEdit(QWidget* parent)
  : QTextEdit(parent)
  {
  _completer = nullptr;
  _clicked_contact = new bts::addressbook::wallet_contact();

  connect(this, &QTextEdit::textChanged, this, &ContactListEdit::fitHeightToDocument);

  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
  fitHeightToDocument();

  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  setTabChangesFocus(true);
  }

ContactListEdit::~ContactListEdit()
  {}

void ContactListEdit::setCompleter(QCompleter* completer)
  {
  if (_completer)
    QObject::disconnect(_completer, 0, this, 0);
  _completer = completer;

  if (!_completer)
    return;

  _completer->setWidget(this);
  _completer->setCompletionMode(QCompleter::PopupCompletion);
  _completer->setCaseSensitivity(Qt::CaseInsensitive);
  
  _completions.clear();
  for (int i = 0; _completer->setCurrentRow(i); i++)
  {
    _completions.push_back(_completer->currentCompletion());
  }
  _completer->setCurrentRow(-1);

   connect(_completer, SIGNAL(activated(const QModelIndex&)),
           this, SLOT(insertCompletion(const QModelIndex&)));
  }

void ContactListEdit::insertCompletion( const QModelIndex& completionIndex )
  {
  if( !completionIndex.isValid())
    return;
  QString completion = completionIndex.data().toString();
  int row = completionIndex.data(Qt::UserRole).toInt();
  row = row / 2;
  //DLNFIX consider telling ContactListEdit about addressbookmodel some other way eventually
  QModelIndex index = getKeyhoteeWindow()->getAddressBookModel()->index(row,0);
  auto contact = getKeyhoteeWindow()->getAddressBookModel()->getContact(index);
  insertCompletion(completion, contact);
  }

void ContactListEdit::insertCompletion( const QString& completion, const bts::addressbook::contact& c)
  {
  ilog( "insertCompletion ${c}", ("c", completion.toStdString() ) );
  // remove existing text
  // create image, attach meta data for on-click menus

  if (_completer->widget() != this)
    return;

  QTextCursor text_cursor = textCursor();
  uint32_t    prefix_len = 0;
  if (_completer->completionPrefix().length() > 0 && _prefixToDelete.length() > 0)
    prefix_len = _prefixToDelete.length();
  for (uint32_t i = 0; i < prefix_len; ++i)
    text_cursor.deletePreviousChar();

  addContactEntry(completion, c);
  }

void ContactListEdit::onCompleterRequest()
  {
  setFocus();
  showCompleter(QString());
  if(_completer->model()->rowCount() == 0)
    {
    QRect pos = cursorRect();
    QToolTip::showText(mapToGlobal(pos.topLeft()), tr("There is no contact defined"));
    }
  }

//! [5]
QString ContactListEdit::textUnderCursor() const
  {
  QTextCursor text_cursor = textCursor();

  int position_of_cursor = text_cursor.position();

  if (! this->toPlainText().isEmpty())
  {
    do {
      text_cursor.movePosition ( QTextCursor::Left, QTextCursor::MoveAnchor);
      int pos_cursor = text_cursor.position();
      QChar prev_char = this->toPlainText().at(pos_cursor);
      if (prev_char==QChar(QChar::ObjectReplacementCharacter))
        {
        text_cursor.movePosition ( QTextCursor::Right, QTextCursor::MoveAnchor);
        break;
        }
    } while(text_cursor.position() != 0);
  }

  text_cursor.setPosition(position_of_cursor, QTextCursor::KeepAnchor);

  QString compilation_prefix = text_cursor.selectedText();

  compilation_prefix = Utils::lTrim(compilation_prefix);

  _completer->setCompletionPrefix(compilation_prefix);
  _prefixToDelete = compilation_prefix;

  if (_completer->completionCount() == 0 && !compilation_prefix.isEmpty())
  {
    for (const QString& completion : _completions)
    {
      if (completion.contains(compilation_prefix))
      {
        _completer->setCompletionPrefix(completion);
        return completion;
      }
    }
  }

  return compilation_prefix;
  }

void ContactListEdit::addContactEntry(const QString& contactText, const bts::addressbook::contact& c)
  {
  QFont        default_font;
  default_font.setPointSize( default_font.pointSize() - 1 );
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
  QPen  pen;

  bool isKeyhoteeFounder = Contact::isKeyhoteeFounder(c);

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
    brush.setColor( QColor( 205, 220, 241 ) );
    pen.setColor( QColor( 105,110,180 ) );
    }

  painter.setBrush(brush);
  painter.setPen(pen);
  painter.drawRoundedRect(0, 0, completion_width - 1, completion_image.height() - 1, 8, 8,
    Qt::AbsoluteSize);
  painter.setPen(QPen());
  painter.drawText(QPoint(10, completion_height - 2), contactText);

  QTextDocument* doc = document();
  doc->addResource(QTextDocument::ImageResource, QUrl(contactText), completion_image);
  QTextImageFormat format;
  format.setName(contactText);

  encodePublicKey(c.public_key, &format);

  QTextCursor txtCursor = textCursor();
  txtCursor.insertImage(format);

  txtCursor.insertText(" ");
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
  bool isShortcut = ((key_event->modifiers() & Qt::ControlModifier) && key_event->key() == Qt::Key_E);   // CTRL+E
  if (!_completer || !isShortcut)   // do not process the shortcut when we have a completer
    QTextEdit::keyPressEvent(key_event);
  //! [7]

  //! [8]
  const bool ctrlOrShift = key_event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
  if (!_completer || (ctrlOrShift && key_event->text().isEmpty()))
    return;

  bool           hasModifier = (key_event->modifiers() != Qt::NoModifier) && !ctrlOrShift;
  QString        completionPrefix = textUnderCursor();

  if (!isShortcut && (hasModifier || key_event->text().isEmpty() || completionPrefix.length() == 0 ))
    {
    _prefixToDelete.clear();
    _completer->popup()->hide();
    return;
    }

  showCompleter(completionPrefix);
  }

void ContactListEdit::showCompleter(const QString& completionPrefix)
  {
  if (completionPrefix != _completer->completionPrefix())
    {
    _completer->setCompletionPrefix(completionPrefix);
    _completer->popup()->setCurrentIndex(_completer->completionModel()->index(0, 0));
    }
  QRect cr = cursorRect();
  cr.setWidth(_completer->popup()->sizeHintForColumn(0)
              + _completer->popup()->verticalScrollBar()->sizeHint().width());
  _completer->complete(cr);   // popup it up!
  }

void ContactListEdit::SetCollectedContacts(const IMailProcessor::TRecipientPublicKeys& storage)
{
  for(const auto& recipient : storage)
  {
    assert(recipient.valid());
    bts::addressbook::contact matchingContact;
    QString entryText(Utils::toString(recipient, Utils::TContactTextFormatting::FULL_CONTACT_DETAILS,
      &matchingContact));
    addContactEntry(entryText, matchingContact);
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
  _right_click = event->pos();
  
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
    if(isClickOnContact())
    {
      if(isStoredContact())
      {
        action_find_contact->setEnabled(true);
      }
      else
      {
        action_add_contact->setEnabled(true);
      }
    }
  }
  
  menu->exec(event->globalPos());
  delete menu;
}

bool ContactListEdit::isClickOnContact()
{
  _right_click += QPoint(2, 0);
  int current_position = this->document()->documentLayout()->hitTest( _right_click, Qt::ExactHit );

  if(current_position > -1)
  {
    QTextBlock current_block = document()->begin();
    while(current_block.isValid())
    {
      for(QTextBlock::iterator it = current_block.begin(); !(it.atEnd()); ++it)
      {
        QTextFragment current_fragment = it.fragment();
        if(current_fragment.isValid())
          if(current_fragment.contains(current_position))
          {
            QTextFormat current_format = current_fragment.charFormat();
            if(current_format.isImageFormat())
            {
              _image_format = current_format.toImageFormat();
              return true;
            }
          }
      }
      current_block = current_block.next();
    }
  }

  return false;
}

bool ContactListEdit::isStoredContact()
{
  if(_image_format.isValid())
  {
    IMailProcessor::TRecipientPublicKey pk;
    decodePublicKey(_image_format, &pk);
    if(Utils::matchContact(pk, _clicked_contact))
      return true;
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
  getKeyhoteeWindow()->openContactGui(_clicked_contact->wallet_index);
  getKeyhoteeWindow()->activateMainWindow();
}
