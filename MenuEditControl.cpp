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

template<class TWidgetClass>
class MenuEditControl::ATextDoc : public ITextDoc
{
public:
  ATextDoc( MenuEditControl* parent )
    : ITextDoc( parent )
  {
  }

protected:
  virtual ~ATextDoc() {};

protected:
  TWidgetClass _focused;

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
  virtual bool canCut()
  {
    return !_focused->isReadOnly();
  }  
};

class MenuEditControl::TextEdit : public ATextDoc<QTextEdit*>
{
public:
  TextEdit( MenuEditControl* parent )
    : ATextDoc( parent )
  {
  }

protected:
  virtual ~TextEdit() {};

private:
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


class MenuEditControl::LineEdit : public ATextDoc<QLineEdit*>
{
public:
  LineEdit( MenuEditControl* parent )
    : ATextDoc( parent )
  {
  }

protected:
  virtual ~LineEdit() {};

private:
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

class MenuEditControl::PlainTextEdit : public ATextDoc<QPlainTextEdit*>
{
public:
  PlainTextEdit( MenuEditControl* parent )
    : ATextDoc( parent )
  {
  }

protected:
  virtual ~PlainTextEdit() {};

private:
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
  _actionPaste (actionPaste),
  _isClosingApp(false)
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
  if (_isClosingApp == true)
    return;

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
  if (_isClosingApp == true)
    return;

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
  if (_isClosingApp == true)
    return;

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
  if (_isClosingApp == true)
    return;

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

void MenuEditControl::onFocusChanged(QWidget *old, QWidget *now)
{  
  if (_isClosingApp == true)
    return;

  bool selectedText = false;
  bool rCanCut = false;
  
  selectedText = isSelected(now, rCanCut);

  if (_actionCopy == nullptr || _actionCut == nullptr || _actionPaste == nullptr)
    return;

  _actionCopy->setEnabled(selectedText);
  _actionCut->setEnabled(rCanCut);
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
  if (_isClosingApp == true)
    return;

  bool rCanCut = false;
  bool selectedText = isSelected(_currentWidget, rCanCut);
  _actionCopy->setEnabled(selectedText);
  _actionCut->setEnabled(rCanCut);
}

void MenuEditControl::onDestroyed( QObject * obj )
{
  _currentWidget = nullptr;
}

bool MenuEditControl::isSelected(QWidget* focused, bool& canCut) const
{
  bool selectedText = false;
  canCut = false;

  if (focused == nullptr)
  {
    return false;
  }

  //contact list
  if(focused == getKeyhoteeWindow()->getContactsPage()->getContactsTableWidget())
  {    
    //enable/disable menu in the: void KeyhoteeMainWindow::refreshMenuOptions() 
    selectedText = true;     
  }
  else
  {
    foreach(ITextDoc* doc, _textDocs)
    {
      if (doc->initWidget(focused))
      {
        selectedText = doc->isSelected();
        canCut = doc->canCut() && selectedText;
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

void MenuEditControl::onClosingApp()
{
  _isClosingApp = true;
}