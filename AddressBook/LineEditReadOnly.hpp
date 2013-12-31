#pragma once
#include <QLineEdit>

class LineEditReadOnly : public QLineEdit
{
    Q_OBJECT
public:
  explicit LineEditReadOnly(QWidget *parent = nullptr);
  virtual ~LineEditReadOnly();

  virtual void setReadOnly(bool); //override
  // bool isReadOnly() const; always return false

protected:
  virtual void mousePressEvent(QMouseEvent*);
  virtual void keyPressEvent(QKeyEvent*);
  virtual void contextMenuEvent(QContextMenuEvent *);
   
private:
  bool _readOnly;
};