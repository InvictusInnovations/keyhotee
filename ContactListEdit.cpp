#include "ContactListEdit.hpp"
#include <QCompleter>
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QScrollBar>
#include <QPainter>

#include <fc/log/logger.hpp>
#include <bts/profile.hpp>
#include "AddressBook/Contact.hpp"

ContactListEdit::ContactListEdit( QWidget* parent )
:QTextEdit(parent)
{
   _completer = nullptr;

   connect( this, &QTextEdit::textChanged, this, &ContactListEdit::fitHeightToDocument ); 

   setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
   fitHeightToDocument();

   setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
   setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
   setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

   setTabChangesFocus(true);
}

ContactListEdit::~ContactListEdit()
{
}

void ContactListEdit::setCompleter( QCompleter* completer )
{
   if( _completer )
   {
      QObject::disconnect(_completer, 0, this, 0 );
   }
   _completer = completer;

   if( !_completer ) return;

   _completer->setWidget(this);
   _completer->setCompletionMode( QCompleter::PopupCompletion );
   _completer->setCaseSensitivity(Qt::CaseInsensitive);
  
   connect(_completer, SIGNAL(activated(const QModelIndex&)),
           this, SLOT(insertCompletion(const QModelIndex&)));

}


QCompleter* ContactListEdit::getCompleter()
{
    return _completer;
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
  bool isKeyhoteeFounder = false;
  if( Contact(contacts[row]).getAge() == 1 )
    isKeyhoteeFounder = true;  

  insertCompletion( completion, isKeyhoteeFounder );
}

void ContactListEdit::insertCompletion( const QString& completion, bool isKeyhoteeFounder )
{
  ilog( "insertCompletion ${c}", ("c", completion.toStdString() ) );
  // remove existing text
  // create image, attach meta data for on-click menus

    if (_completer->widget() != this)
        return;
    QFont        default_font;
    default_font.setPointSize( default_font.pointSize() - 1 );
    QFontMetrics font_metrics(default_font);
    QRect        bounding = font_metrics.boundingRect( completion );
    int          completion_width = font_metrics.width( completion );
    int          completion_height = bounding.height();

    completion_width += 20;

    QImage completion_image( completion_width, completion_height+4, QImage::Format_ARGB32 );
    completion_image.fill( QColor( 0,0,0,0 ) );
    QPainter painter;
    painter.begin(&completion_image);
    painter.setFont(default_font);
    painter.setRenderHint( QPainter::Antialiasing );
    QBrush brush(Qt::SolidPattern);
    QPen pen;

    if(isKeyhoteeFounder)
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

    painter.setBrush( brush );
    painter.setPen(pen);
    painter.drawRoundedRect( 0, 0, completion_width-1, completion_image.height()-1, 8, 8, Qt::AbsoluteSize );
    painter.setPen(QPen());
    painter.drawText( QPoint( 10, completion_height - 2 ), completion );

    QTextCursor text_cursor = textCursor();
    uint32_t prefix_len =  _completer->completionPrefix().length();
    for( uint32_t i = 0; i < prefix_len; ++i )
    {
        text_cursor.deletePreviousChar();
    }
   // int extra = completion.length() -
   // tc.movePosition(QTextCursor::Left);
   // tc.movePosition(QTextCursor::EndOfWord);
   // tc.insertText(completion.right(extra));
    text_cursor.insertImage( completion_image, completion );
    text_cursor.insertText(" ");
    setTextCursor(text_cursor);
}

//! [5]
QString ContactListEdit::textUnderCursor() const
{
    QTextCursor text_cursor = textCursor();
    text_cursor.select(QTextCursor::WordUnderCursor);
    return text_cursor.selectedText();
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
   if ( _completer && _completer->popup()->isVisible()) 
   {
      return false;
   }
   return QTextEdit::focusNextPrevChild(next);
}

//! [7]
void ContactListEdit::keyPressEvent(QKeyEvent* key_event)
{
    if ( _completer && _completer->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
       switch (key_event->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            key_event->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
    }
    bool isShortcut = ((key_event->modifiers() & Qt::ControlModifier) && key_event->key() == Qt::Key_E); // CTRL+E
    if (!_completer || !isShortcut) // do not process the shortcut when we have a completer
        QTextEdit::keyPressEvent(key_event);
//! [7]

//! [8]
    const bool ctrlOrShift = key_event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!_completer || (ctrlOrShift && key_event->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    bool hasModifier = (key_event->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || key_event->text().isEmpty()|| completionPrefix.length() == 0
                      || eow.contains(key_event->text().right(1)))) {
        _completer->popup()->hide();
        return;
    }

    if (completionPrefix != _completer->completionPrefix()) {
        _completer->setCompletionPrefix(completionPrefix);
        _completer->popup()->setCurrentIndex(_completer->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(_completer->popup()->sizeHintForColumn(0)
                + _completer->popup()->verticalScrollBar()->sizeHint().width());
    _completer->complete(cr); // popup it up!
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

void ContactListEdit::resizeEvent( QResizeEvent* resize_event )
{
    fitHeightToDocument();
    QTextEdit::resizeEvent(resize_event);
}
