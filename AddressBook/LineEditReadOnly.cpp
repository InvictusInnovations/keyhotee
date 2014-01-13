#include "AddressBook/LineEditReadOnly.hpp"
#include <QKeyEvent>
#include <QMenu>

LineEditReadOnly::LineEditReadOnly(QWidget* parent)
  : QLineEdit(parent)
  {
  _readOnly = true;
  }

LineEditReadOnly::~LineEditReadOnly(){}


void LineEditReadOnly::setReadOnly(bool enabled)
  {
  _readOnly = enabled;  
  //do not call, because Ctrl+C doesn't work
  //QLineEdit::setReadOnly(enabled);
  }

void LineEditReadOnly::mousePressEvent(QMouseEvent *mouse_event)
  {
//  if (_readOnly)
//    selectAll(); //This is not standard behavior
//  else
    QLineEdit::mousePressEvent(mouse_event);
  }

void LineEditReadOnly::keyPressEvent(QKeyEvent* key_event)
  {
  if (! _readOnly)
    return QLineEdit::keyPressEvent(key_event);

  bool isCtrlC = ((key_event->modifiers() & Qt::ControlModifier) && key_event->key() == Qt::Key_C); // Ctrl+C
  bool isCtrlA = ((key_event->modifiers() & Qt::ControlModifier) && key_event->key() == Qt::Key_A); // Ctrl+A
  if (isCtrlC || isCtrlA)
    {
    return QLineEdit::keyPressEvent(key_event);
    }
  else
    return;
  }

 void LineEditReadOnly::contextMenuEvent(QContextMenuEvent *event)
  {
  if (! _readOnly)
    return QLineEdit::contextMenuEvent(event);

  #ifndef QT_NO_CLIPBOARD    
  QMenu *menu = new QMenu();
  QAction *actionCopy = menu->addAction(QLineEdit::tr("&Copy"));
  menu->addSeparator ();
  QAction *actionSelectAll = menu->addAction(QLineEdit::tr("&Select All"));
  actionCopy->setEnabled(hasSelectedText());
  connect(actionCopy, SIGNAL(triggered()), SLOT(copy()));
  connect(actionSelectAll, SIGNAL(triggered()), SLOT(selectAll())); 
  menu->exec(event->globalPos());
  delete menu;
  #endif
  }