#pragma once

#include "ShowColumnAction.hpp"

#include <QTableView>


class MailTable : public QTableView
{
    Q_OBJECT
public:
  explicit MailTable(QWidget *parent = nullptr);
  virtual ~MailTable() {};

  void initialActions();
  
private slots:
  /// \see ShowColumnAction::showColumn signal description.
  void onShowColumn(bool visible, MailboxModel::Columns column);

private:
  void setupActions();
  /// Create action and add to horizontal header
  void initializeAction(QString columnName, 
                        MailboxModel::Columns columnType);

};