#include "AddressBook/LineEditReadOnly.hpp"
#include <QKeyEvent>

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
    QLineEdit::keyPressEvent(key_event);

  bool isCtrlC = ((key_event->modifiers() & Qt::ControlModifier) && key_event->key() == Qt::Key_C); // Ctrl+C
  if (isCtrlC)
    {
    QLineEdit::keyPressEvent(key_event);
    }
  else
    return;
  }