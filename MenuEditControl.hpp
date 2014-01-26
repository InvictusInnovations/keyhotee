#ifndef MENUEDITCONTROL_HPP
#define MENUEDITCONTROL_HPP

#include <QObject>

class QAction;
class QWidget;

class MenuEditControl : public QObject
{
  Q_OBJECT
public:
  explicit MenuEditControl(QObject *parent, QAction *actionCopy, QAction *actionCut);
  void copy() const;
  void cut();
  void paste();
  void selectAll();
  void setEnabled(QWidget *old, QWidget *now);

private:
  bool isSelected(QWidget* focused) const;
  void connectSelectionChangedSignal(bool fConnect, QWidget* widget);

signals:
private slots:
  void onSelectionChanged();

private:
  QAction*    _actionCopy;
  QAction*    _actionCut;
  QWidget *   _currentWidget;
};

#endif // MENUEDITCONTROL_HPP
