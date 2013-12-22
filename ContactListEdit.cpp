#include "ContactListEdit.hpp"

#include "public_key_address.hpp"
#include "AddressBook/Contact.hpp"

#include <QCompleter>
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QScrollBar>
#include <QPainter>
#include <QTextBlock>

#include <fc/log/logger.hpp>
#include <bts/profile.hpp>

ContactListEdit::ContactListEdit(QWidget* parent)
  : QTextEdit(parent)
  {
  _completer = nullptr;

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
  auto addressbook = bts::get_profile()->get_addressbook();
  auto contacts = addressbook->get_contacts();
  auto contact = contacts[row];
  
  insertCompletion(completion, contact);
  }

void ContactListEdit::insertCompletion( const QString& completion, const bts::addressbook::contact& c)
  {
  ilog( "insertCompletion ${c}", ("c", completion.toStdString() ) );
  // remove existing text
  // create image, attach meta data for on-click menus

  if (_completer->widget() != this)
    return;

  addContactEntry(completion, c);
  }

void ContactListEdit::onCompleterRequest()
  {
  setFocus();
  showCompleter(QString());
  }

//! [5]
QString ContactListEdit::textUnderCursor() const
  {
  QTextCursor text_cursor = textCursor();
  text_cursor.select(QTextCursor::WordUnderCursor);
  return text_cursor.selectedText();
  }

QStringList ContactListEdit::getListOfImageNames() const
  {
  QStringList image_names;
  QTextBlock  block = document()->begin();
  while (block.isValid())
    {
    for (QTextBlock::iterator i = block.begin(); !i.atEnd(); ++i)
      {
      QTextCharFormat format = i.fragment().charFormat();
      bool            isImage = format.isImageFormat();
      if (isImage)
        image_names << format.toImageFormat().name();
      }
    block = block.next();
    }
  return image_names;
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
    grad.setColorAt(0.3, QColor(231, 190, 66));
    grad.setColorAt(1.0, QColor(103, 51, 1));
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

  QTextCursor text_cursor = textCursor();
  uint32_t    prefix_len = _completer->completionPrefix().length();
  for (uint32_t i = 0; i < prefix_len; ++i)
    text_cursor.deletePreviousChar();
     // int extra = completion.length() -
  // tc.movePosition(QTextCursor::Left);
  // tc.movePosition(QTextCursor::EndOfWord);
  // tc.insertText(completion.right(extra));
  text_cursor.insertImage(completion_image, contactText);
  text_cursor.insertText(" ");
  setTextCursor(text_cursor);
  }

QString ContactListEdit::toString(const bts::addressbook::wallet_identity& id) const
  {
  return toStringImpl(id);
  }

QString ContactListEdit::toString(const bts::addressbook::wallet_contact& id) const
  {
  return id.getDisplayName().c_str();//toStringImpl(id);
  }

template <class TContactStorage>
inline
QString ContactListEdit::toStringImpl(const TContactStorage& id) const
  {
  /** Use trivial encoding (as kID only) until some underlying storage for added contact
      data will be implemented. Right now built text must allow successfull reverse parsing
      what is completely wrong
  */
  bool noAlias = true;//id.first_name.empty() && id.last_name.empty();
  std::string identity_label;
  if(noAlias == false)
    identity_label = id.first_name + " " + id.last_name;

  std::string entry(identity_label);

  if(noAlias == false)
    entry += '(';

  entry += id.dac_id_string;

  if(noAlias == false)
    entry += ')';

  return QString(entry.c_str());
  }

//! [5]

//! [6]
void ContactListEdit::focusInEvent(QFocusEvent* focus_event)
  {
  if (_completer)
    _completer->setWidget(this);
  QTextEdit::focusInEvent(focus_event);
  }

//! [6]

bool ContactListEdit::focusNextPrevChild(bool next)
  {
  if (_completer && _completer->popup()->isVisible())
    return false;
  return QTextEdit::focusNextPrevChild(next);
  }

//! [7]
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

  static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");   // end of word
  bool           hasModifier = (key_event->modifiers() != Qt::NoModifier) && !ctrlOrShift;
  QString        completionPrefix = textUnderCursor();

  if (!isShortcut && (hasModifier || key_event->text().isEmpty() || completionPrefix.length() == 0
                      || eow.contains(key_event->text().right(1))))
    {
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
  auto profile = bts::get_profile();
  auto aBook = profile->get_addressbook();
  for(const auto& recipient : storage)
    {
    assert(recipient.valid());

    auto contact = aBook->get_contact_by_public_key(recipient);

    if(contact)
      {
      /** Use kID as completion here - it is slight violation against source list but we don't know
          here how it was originally entered (by kID or alias: fName lName)
      */
      QString entryText(toString(*contact));
      addContactEntry(entryText, *contact);
      }
    else
      {
      bool known = false;
      /// If no contact found try one of registered identities.
      std::vector<bts::addressbook::wallet_identity> identities = profile->identities();
      for(const auto& identity : identities)
        {
        assert(identity.public_key.valid());

        if(identity.public_key == recipient)
          {
          QString entryText(toString(identity));
          addContactEntry(entryText, identity);
          known = true;
          break;
          }
        }

      if(known == false)
        {
        /// Some unknown contact. Lets show its public key in the list.
        public_key_address pkAddress(recipient);
        std::string textPK(pkAddress);
        bts::addressbook::wallet_contact wc;
        wc.public_key = recipient;
        addContactEntry(QString(textPK.c_str()), wc);
        }
      }
    }
  }

void ContactListEdit::GetCollectedContacts(IMailProcessor::TRecipientPublicKeys* storage) const
  {
  /** FIXME - this code generally looks bad. It will don't work when in AB will be conflict between
      kID and fName lName alias.
      Whole completer <-> contact edit communication is wrong, since it should operate on Contact
      objects not just strings (where noone knows it is a kID or alias).
      Another problem is synchronization against AB changes. It is also did wrong. To do it correclty
      instantiated completer should use dedicated model operating DIRECTLY on AB not copied string
      list.
  */
  auto addressbook = bts::get_profile()->get_addressbook();
  QStringList recipient_image_names = getListOfImageNames();
  storage->reserve(recipient_image_names.size());

  for(const auto& recipient : recipient_image_names)
    {
    std::string to_string = recipient.toStdString();
    //check first to see if we have a dac_id
    auto to_contact = addressbook->get_contact_by_dac_id(to_string);
    if (!to_contact.valid()) // if not dac_id, check if we have a full name
      to_contact = addressbook->get_contact_by_display_name(to_string);
    assert(to_contact.valid());
    storage->push_back(to_contact->public_key);
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

