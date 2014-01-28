#include "MenuEditControl.hpp"
#include "KeyhoteeMainWindow.hpp"
#include "AddressBook/ContactsTable.hpp"

#include <QAction>
#include <QLineEdit>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTableView>
#include <QWidget>


class MenuEditControl::TextEdit : public ITextDoc
{
  QTextEdit* _focused;

  virtual bool initWidget(QWidget* focused)
  {
    if(focused->inherits("QTextEdit"))
    {
      _focused = qobject_cast<QTextEdit*>(focused);
      return true;
    }
    return false;
  }

  virtual void copy()
  {
    _focused->copy();
  }
  virtual void cut()
  {
    _focused->cut();
  }
  virtual void paste()
  {
    _focused->paste();
  }
  virtual void selectAll()
  {
    _focused->selectAll();
  }
  virtual bool isSelected()
  {
    return !_focused->textCursor().selectedText().isEmpty();
  }
};


class MenuEditControl::LineEdit : public ITextDoc
{
  QLineEdit* _focused;

  virtual bool initWidget(QWidget* focused)
  {
    if(focused->inherits("QLineEdit"))
    {
      _focused = qobject_cast<QLineEdit*>(focused);
      return true;
    }
    return false;
  }

  virtual void copy()
  {
    _focused->copy();
  }
  virtual void cut()
  {
    _focused->cut();
  }
  virtual void paste()
  {
    _focused->paste();
  }
  virtual void selectAll()
  {
    _focused->selectAll();
  }
  virtual bool isSelected()
  {
    return !_focused->selectedText().isEmpty();
  }
};

class MenuEditControl::PlainTextEdit : public ITextDoc
{
  QPlainTextEdit* _focused;

  virtual bool initWidget(QWidget* focused)
  {
    if(focused->inherits("QPlainTextEdit"))
    {
      _focused = qobject_cast<QPlainTextEdit*>(focused);
      return true;
    }
    return false;
  }

  virtual void copy()
  {
    _focused->copy();
  }
  virtual void cut()
  {
    _focused->cut();
  }
  virtual void paste()
  {
    _focused->paste();
  }
  virtual void selectAll()
  {
    _focused->selectAll();
  }
  virtual bool isSelected()
  {
    return !_focused->textCursor().selectedText().isEmpty();
  }
};

MenuEditControl::MenuEditControl(QObject *parent, QAction *actionCopy, QAction *actionCut)
:  QObject(parent), 
  _actionCopy (actionCopy),
  _actionCut (actionCut)
{
  _currentWidget = nullptr;
  _textDocs.push_back(new MenuEditControl::TextEdit() );
  _textDocs.push_back(new MenuEditControl::LineEdit() );
  _textDocs.push_back(new MenuEditControl::PlainTextEdit() );
}

MenuEditControl::~MenuEditControl()
{
  for (size_t i = 0; i < _textDocs.size(); i++)
    delete _textDocs[i];
}

void MenuEditControl::copy() const
{
  KeyhoteeMainWindow* mainWin = getKeyhoteeWindow();
  QWidget *focused = mainWin->focusWidget ();

  if (focused == nullptr)
    return;

  foreach(ITextDoc* doc, _textDocs)
  {
    if (doc->initWidget(focused))
    {
      doc->copy();
      break;
    }
  }
}

void MenuEditControl::cut()
{
  KeyhoteeMainWindow* mainWin = getKeyhoteeWindow();
  QWidget *focused = mainWin->focusWidget ();

  if (focused == nullptr)
    return;

  foreach(ITextDoc* doc, _textDocs)
  {
    if (doc->initWidget(focused))
    {
      doc->cut();
      break;
    }
  }
}

void MenuEditControl::paste()
{
  KeyhoteeMainWindow* mainWin = getKeyhoteeWindow();
  QWidget *focused = mainWin->focusWidget ();

  if (focused == nullptr)
    return;

  foreach(ITextDoc* doc, _textDocs)
  {
    if (doc->initWidget(focused))
    {
      doc->paste();
      break;
    }
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
  else
  {
    foreach(ITextDoc* doc, _textDocs)
    {
      if (doc->initWidget(focused))
      {
        doc->selectAll();
        break;
      }
    }
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

  //contact list
  if(focused == getKeyhoteeWindow()->getContactsPage()->getContactsTableWidget())
  {    
    //enable/disable menu in the: void KeyhoteeMainWindow::refreshEditMenu() 
    selectedText = true; 
  }
  else
  {
    foreach(ITextDoc* doc, _textDocs)
    {
      if (doc->initWidget(focused))
      {
        selectedText = doc->isSelected();
        break;
      }
    }  
  }

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