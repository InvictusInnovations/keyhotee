#pragma once
#include <QTableView>

class IModificationsChecker;

class TableViewCustom : public QTableView
{
    Q_OBJECT
public:
  explicit TableViewCustom(QWidget *parent = nullptr);
  virtual ~TableViewCustom();
  void setModificationsChecker (IModificationsChecker*);

protected:
  virtual bool viewportEvent(QEvent *event) override;
  
private:
  IModificationsChecker*    _modificationsChecker;
};