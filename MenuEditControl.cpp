#include "MenuEditControl.hpp"
#include "KeyhoteeMainWindow.hpp"
#include "AddressBook/ContactsTable.hpp"

#include <QAction>
#include <QLineEdit>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTableView>
#include <QWidget>

MenuEditControl::ITextDoc::ITextDoc( MenuEditControl* parent )
{
  _parent = parent;
  _focused = nullptr;  
}

class MenuEditControl::TextEdit : public ITextDoc
{
public:
  TextEdit::TextEdit( MenuEditControl* parent )
    : ITextDoc( parent )
  {
  }
  virtual ~TextEdit() {};

private:
  QTextEdit* _focused;

  virtual bool initWidget(QWidget* focused)
  {
    if(focused->inherits("QTextEdit"))
    {
      _focused = qobject_cast<QTextEdit*>(focused);
      return true;
    }
    else
      _focused = nullptr;
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
  virtual bool canPaste()
  {
    return _focused->canPaste();
  }
  virtual void connectSelectionChanged(bool fConnect, QWidget* widget)
  {
    if (fConnect)
      _parent->connect(qobject_cast<QTextEdit*>(widget),  &QTextEdit::selectionChanged, _parent, &MenuEditControl::onSelectionChanged);
    else
      _parent->disconnect(qobject_cast<QTextEdit*>(widget),  &QTextEdit::selectionChanged, _parent, &MenuEditControl::onSelectionChanged);
  }
};


class MenuEditControl::LineEdit : public ITextDoc
{
public:
  LineEdit::LineEdit( MenuEditControl* parent )
    : ITextDoc( parent )
  {
  }
  virtual ~LineEdit() {};

private:
  QLineEdit* _focused;

  virtual bool initWidget(QWidget* focused)
  {
    if(focused->inherits("QLineEdit"))
    {
      _focused = qobject_cast<QLineEdit*>(focused);
      return true;
    }
    else
      _focused = nullptr;
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
  virtual bool canPaste()
  {
   return  !_focused->isReadOnly();
  }
  virtual void connectSelectionChanged(bool fConnect, QWidget* widget)
  {
    if (fConnect)
      _parent->connect(qobject_cast<QLineEdit*>(widget),  &QLineEdit::selectionChanged, _parent, &MenuEditControl::onSelectionChanged);
    else
      _parent->disconnect(qobject_cast<QLineEdit*>(widget),  &QLineEdit::selectionChanged, _parent, &MenuEditControl::onSelectionChanged);
  }
};

class MenuEditControl::PlainTextEdit : public ITextDoc
{
public:
  PlainTextEdit::PlainTextEdit( MenuEditControl* parent )
    : ITextDoc( parent )
  {
  }
  virtual ~PlainTextEdit() {};

private:
  QPlainTextEdit* _focused;

  virtual bool initWidget(QWidget* focused)
  {
    if(focused->inherits("QPlainTextEdit"))
    {
      _focused = qobject_cast<QPlainTextEdit*>(focused);
      return true;
    }
    else
      _focused = nullptr;
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
  virtual bool canPaste()
  {
    return _focused->canPaste();
  }
  virtual void connectSelectionChanged(bool fConnect, QWidget* widget)
  {
    if (fConnect)
      _parent->connect(qobject_cast<QPlainTextEdit*>(widget),  &QPlainTextEdit::selectionChanged, _parent, &MenuEditControl::onSelectionChanged);
    else
      _parent->disconnect(qobject_cast<QPlainTextEdit*>(widget),  &QPlainTextEdit::selectionChanged, _parent, &MenuEditControl::onSelectionChanged);
  }
};

MenuEditControl::MenuEditControl(QObject *parent, QAction *actionCopy, QAction *actionCut, QAction *actionPaste)
:  QObject(parent), 
  _actionCopy (actionCopy),
  _actionCut (actionCut),
  _actionPaste (actionPaste)
{
  _currentWidget = nullptr;
  _textDocs.push_back(new MenuEditControl::TextEdit(this) );
  _textDocs.push_back(new MenuEditControl::LineEdit(this) );
  _textDocs.push_back(new MenuEditControl::PlainTextEdit(this) );
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
  _actionPaste->setEnabled(false);

  if (now == nullptr)
    return;

  foreach(ITextDoc* doc, _textDocs)
  {
    if (doc->initWidget(now))
    {
      _actionPaste->setEnabled(doc->canPaste());
      break;
    }
  }

  if (now != _currentWidget)
  {
    connectSelectionChangedSignal(false, _currentWidget);    
    if (connectSelectionChangedSignal(true, now))
      _currentWidget = now;
    else
      _currentWidget = nullptr;
  }
}

void MenuEditControl::onSelectionChanged()
{
  bool selectedText = isSelected(_currentWidget);
  _actionCopy->setEnabled(selectedText);
  _actionCut->setEnabled(selectedText);
}

void MenuEditControl::onDestroyed( QObject * obj )
{
  _currentWidget = nullptr;
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

bool MenuEditControl::connectSelectionChangedSignal(bool fConnect, QWidget* widget)
{
  if (widget == nullptr)
    return false;

  foreach(ITextDoc* doc, _textDocs)
  {
    if (doc->initWidget(widget))
    {
      if (fConnect)
        connect(widget,  &QObject::destroyed, this, &MenuEditControl::onDestroyed);
      else
        disconnect(widget,  &QObject::destroyed, this, &MenuEditControl::onDestroyed);

      doc->connectSelectionChanged( fConnect, widget );
      return true;
    }
  }

  return false;
}