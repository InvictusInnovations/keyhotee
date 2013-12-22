#pragma once
#include <QTreeWidget>

class IModificationsChecker;

class TreeWidgetCustom : public QTreeWidget
{
    Q_OBJECT
public:
  explicit TreeWidgetCustom(QWidget *parent = nullptr);
  virtual ~TreeWidgetCustom();
  void setModificationsChecker (IModificationsChecker*);

protected:
  virtual bool viewportEvent(QEvent *event) override;

private:
  IModificationsChecker*  _modificationsChecker;
};