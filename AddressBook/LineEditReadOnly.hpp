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
  void mousePressEvent(QMouseEvent*);
  void keyPressEvent(QKeyEvent*);
   
private:
  bool _readOnly;
};