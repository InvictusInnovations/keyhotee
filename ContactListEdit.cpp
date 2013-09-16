#include "ContactListEdit.hpp"
#include <QCompleter>
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QScrolLBar>

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
  
   connect(_completer, SIGNAL(activated(QString)),
           this, SLOT(insertCompletion(QString)));

}


QCompleter* ContactListEdit::getCompleter()
{
    return _completer;
}


void ContactListEdit::insertCompletion( const QString& completion )
{
  // remove existing text
  // create image, attach meta data for on-click menus

    if (_completer->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - _completer->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
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

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 3
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
