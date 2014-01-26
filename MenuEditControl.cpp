#include "MenuEditControl.hpp"
#include "KeyhoteeMainWindow.hpp"
#include "AddressBook/ContactsTable.hpp"

#include <QAction>
#include <QLineEdit>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTableView>
#include <QWidget>


MenuEditControl::MenuEditControl(QObject *parent, QAction *actionCopy, QAction *actionCut)
:  QObject(parent), 
  _actionCopy (actionCopy),
  _actionCut (actionCut)
{
  _currentWidget = nullptr;
}

void MenuEditControl::copy() const
{
  KeyhoteeMainWindow* mainWin = getKeyhoteeWindow();
  QWidget *focused = mainWin->focusWidget ();

  if (focused == nullptr)
    return;

  if(focused->inherits("QTextEdit")) 
  {
    qobject_cast<QTextEdit*>(focused)->copy();
  }
  else if(focused->inherits("QLineEdit")) 
  {
    qobject_cast<QLineEdit*>(focused)->copy();
  }
  else if(focused->inherits("QPlainTextEdit")) 
  {
    qobject_cast<QPlainTextEdit*>(focused)->copy();
  }
  //contact list
  else if(focused == mainWin->getContactsPage()->getContactsTableWidget())
  {
    mainWin->getContactsPage()->copy();
  }
}

void MenuEditControl::cut()
{
  KeyhoteeMainWindow* mainWin = getKeyhoteeWindow();
  QWidget *focused = mainWin->focusWidget ();

  if (focused == nullptr)
    return;

  if(focused->inherits("QTextEdit")) 
  {
    qobject_cast<QTextEdit*>(focused)->cut();
  }
  else if(focused->inherits("QLineEdit")) 
  {
    qobject_cast<QLineEdit*>(focused)->cut();
  }
  else if(focused->inherits("QPlainTextEdit")) 
  {
    qobject_cast<QPlainTextEdit*>(focused)->cut();
  }
}

void MenuEditControl::paste()
{
  KeyhoteeMainWindow* mainWin = getKeyhoteeWindow();
  QWidget *focused = mainWin->focusWidget ();

  if (focused == nullptr)
    return;

  if(focused->inherits("QTextEdit")) 
  {
    qobject_cast<QTextEdit*>(focused)->paste();
  }
  else if(focused->inherits("QLineEdit")) 
  {
    qobject_cast<QLineEdit*>(focused)->paste();
  }
  else if(focused->inherits("QPlainTextEdit")) 
  {
    qobject_cast<QPlainTextEdit*>(focused)->paste();
  }
}

void MenuEditControl::selectAll()
{
  KeyhoteeMainWindow* mainWin = getKeyhoteeWindow();
  QWidget *focused = mainWin->focusWidget ();

  if (focused == nullptr)
    return;

  //contact list, mail list
  if(focused->inherits("QTableView")) 
  {
    qobject_cast<QTableView*>(focused)->selectAll();
  }
  //chat readOnly window
  else if(focused->inherits("QTextEdit")) 
  {
    qobject_cast<QTextEdit*>(focused)->selectAll();
  }
  //public key readOnly window
  else if(focused->inherits("QLineEdit")) 
  {
    qobject_cast<QLineEdit*>(focused)->selectAll();
  }
  else if(focused->inherits("QPlainTextEdit")) 
  {
    qobject_cast<QPlainTextEdit*>(focused)->selectAll();
  }
  else
  {
    assert(0);
  }
}

void MenuEditControl::setEnabled(QWidget *old, QWidget *now)
{  
  bool selectedText = false;
  
  selectedText = isSelected(now);
  _actionCopy->setEnabled(selectedText);
  _actionCut->setEnabled(selectedText);

  if (now == nullptr)
    return;

  if (now != _currentWidget)
  {
    connectSelectionChangedSignal(false, _currentWidget);
    _currentWidget = now;
    connectSelectionChangedSignal(true, _currentWidget);
  }
}

void MenuEditControl::onSelectionChanged()
{
  bool selectedText = isSelected(_currentWidget);
  _actionCopy->setEnabled(selectedText);
  _actionCut->setEnabled(selectedText);
}

bool MenuEditControl::isSelected(QWidget* focused) const
{
  bool selectedText = false;

  if (focused == nullptr)
  {
    return false;
  }

  if(focused->inherits("QTextEdit")) 
  {
    selectedText = ! qobject_cast<QTextEdit*>(focused)->textCursor().selectedText().isEmpty();
  }
  else if(focused->inherits("QLineEdit")) 
  {
    selectedText = ! qobject_cast<QLineEdit*>(focused)->selectedText().isEmpty();
  }
  else if(focused->inherits("QPlainTextEdit")) 
  {
    selectedText = ! qobject_cast<QPlainTextEdit*>(focused)->textCursor().selectedText().isEmpty();
  }
  //contact list
  /*else if(focused == getKeyhoteeWindow()->getContactsPage()->getContactsTableWidget())
  {
    getKeyhoteeWindow()->getContactsPage()->getContactsTableWidget()->isSelected();
  }*/

  return selectedText;
}

void MenuEditControl::connectSelectionChangedSignal(bool fConnect, QWidget* widget)
{
  if (widget == nullptr)
    return;

  if(widget->inherits("QTextEdit")) 
  {
    if (fConnect)
      connect(qobject_cast<QTextEdit*>(widget),  &QTextEdit::selectionChanged, this, &MenuEditControl::onSelectionChanged);
    else
      disconnect(qobject_cast<QTextEdit*>(widget),  &QTextEdit::selectionChanged, this, &MenuEditControl::onSelectionChanged);
  }
  else if(widget->inherits("QLineEdit")) 
  {
    if (fConnect)
      connect(qobject_cast<QLineEdit*>(widget), &QLineEdit::selectionChanged, this, &MenuEditControl::onSelectionChanged);
    else
      disconnect(qobject_cast<QLineEdit*>(widget), &QLineEdit::selectionChanged, this, &MenuEditControl::onSelectionChanged);
  }
  else if(widget->inherits("QPlainTextEdit")) 
  {
    if (fConnect)
      connect(qobject_cast<QPlainTextEdit*>(widget), &QPlainTextEdit::selectionChanged, this, &MenuEditControl::onSelectionChanged);
    else
      disconnect(qobject_cast<QPlainTextEdit*>(widget), &QPlainTextEdit::selectionChanged, this, &MenuEditControl::onSelectionChanged);
  }
}