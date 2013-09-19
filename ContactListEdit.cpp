#include "ContactListEdit.hpp"
#include <QCompleter>
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QScrolLBar>
#include <QPainter>

#include <fc/log/logger.hpp>

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
  
   connect(_completer, SIGNAL(activated(const QString&)),
           this, SLOT(insertCompletion(const QString&)));

}


QCompleter* ContactListEdit::getCompleter()
{
    return _completer;
}


void ContactListEdit::insertCompletion( const QString& completion )
{
   ilog( "insertCompletion ${c}", ("c", completion.toStdString() ) );
  // remove existing text
  // create image, attach meta data for on-click menus

    if (_completer->widget() != this)
        return;
    QFont        default_font;
    default_font.setPointSize( default_font.pointSize() - 2 );
    QFontMetrics font_metrics(default_font);
    QRect        bounding = font_metrics.boundingRect( completion );
    int          comp_width = font_metrics.width( completion );
    int          comp_height = bounding.height();

    comp_width += 20;

    QImage completion_image( comp_width, comp_height+4, QImage::Format_ARGB32 );
    completion_image.fill( QColor( 0,0,0,0 ) );
    QPainter painter;
    painter.begin(&completion_image);
    painter.setFont(default_font);
    painter.setRenderHint( QPainter::Antialiasing );

    QBrush brush(Qt::SolidPattern);
    brush.setColor( QColor( 205, 220, 241 ) );
    QPen pen;
    pen.setColor( QColor( 105,110,180 ) );

    painter.setBrush( brush );
    painter.setPen(pen);
    painter.drawRoundedRect( 0, 0, comp_width-1, completion_image.height()-1, 8, 8, Qt::AbsoluteSize );
    painter.setPen(QPen());
    painter.drawText( QPoint( 10, comp_height - 2 ), completion );

    QTextCursor tc = textCursor();
    uint32_t prefix_len =  _completer->completionPrefix().length();
    for( uint32_t i = 0; i < prefix_len; ++i )
    {
        tc.deletePreviousChar();
    }
   // int extra = completion.length() -
   // tc.movePosition(QTextCursor::Left);
   // tc.movePosition(QTextCursor::EndOfWord);
   // tc.insertText(completion.right(extra));
    tc.insertImage( completion_image, completion );
    tc.insertText(" ");
    setTextCursor(tc);
}

//! [5]
QString ContactListEdit::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}
//! [5]

//! [6]
void ContactListEdit::focusInEvent(QFocusEvent *e)
{
    if (_completer)
        _completer->setWidget(this);
    QTextEdit::focusInEvent(e);
}
//! [6]

//! [7]
void ContactListEdit::keyPressEvent(QKeyEvent *e)
{
    if ( _completer && _completer->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
    if (!_completer || !isShortcut) // do not process the shortcut when we have a completer
        QTextEdit::keyPressEvent(e);
//! [7]

//! [8]
    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!_completer || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() == 0
                      || eow.contains(e->text().right(1)))) {
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
void ContactListEdit::resizeEvent( QResizeEvent* e )
{
    fitHeightToDocument();
    QTextEdit::resizeEvent(e);
}
