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
  if (_readOnly)
    selectAll();
  else
    QLineEdit::mousePressEvent(mouse_event);
  }

void LineEditReadOnly::keyPressEvent(QKeyEvent* key_event)
  {
  if (! _readOnly)
    return QLineEdit::keyPressEvent(key_event);

  bool isCtrlC = ((key_event->modifiers() & Qt::ControlModifier) && key_event->key() == Qt::Key_C); // Ctrl+C
  if (isCtrlC)
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
  QAction *action = menu->addAction(QLineEdit::tr("&Copy"));
  action->setEnabled(hasSelectedText());
  connect(action, SIGNAL(triggered()), SLOT(copy()));
  menu->exec(event->globalPos());
  delete menu;
  #endif
  }