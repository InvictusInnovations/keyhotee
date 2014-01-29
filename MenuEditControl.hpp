#ifndef MENUEDITCONTROL_HPP
#define MENUEDITCONTROL_HPP

#include <QObject>

class QAction;
class QWidget;

class MenuEditControl : public QObject
{
  Q_OBJECT

private:
  class ITextDoc
  {
  public: //delete from MenuEditControl
    ITextDoc(MenuEditControl* parent);
    virtual ~ITextDoc(){};
  public:
    virtual bool initWidget(QWidget* focused) = 0;
    virtual void copy() = 0;
    virtual void cut() = 0;
    virtual void paste() = 0;
    virtual void selectAll() = 0;
    virtual bool isSelected() = 0;
    virtual bool canPaste() = 0;
    virtual void connectSelectionChanged(bool fConnect, QWidget* widget) = 0;
  protected:
    QWidget*          _focused;
    MenuEditControl*  _parent;
  };

  class TextEdit;
  class LineEdit;
  class PlainTextEdit;


public:
  explicit MenuEditControl(QObject *parent, QAction *actionCopy, QAction *actionCut, QAction *actionPaste);
  virtual ~MenuEditControl();
  void copy() const;
  void cut();
  void paste();
  void selectAll();
  void setEnabled(QWidget *old, QWidget *now);

private:
  bool isSelected(QWidget* focused) const;
  bool connectSelectionChangedSignal(bool fConnect, QWidget* widget);

signals:
private slots:
  void onSelectionChanged();
  void onDestroyed ( QObject * obj = 0 );

private:
  QAction*    _actionCopy;
  QAction*    _actionCut;
  QAction*    _actionPaste;
  QWidget *   _currentWidget;
  std::vector<ITextDoc*>   _textDocs;
};

#endif // MENUEDITCONTROL_HPP
