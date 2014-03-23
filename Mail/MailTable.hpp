#pragma once

#include "ShowColumnAction.hpp"

#include <QTableView>

#include "MailboxModel.hpp"

class MailTable : public QTableView
{
    Q_OBJECT
public:
  /// Initial settings for mail table
  struct InitialSettings
  {
    int                           sortColumn;
    Qt::SortOrder                 sortOrder;
    // Columns displayed
    QList<MailboxModel::Columns>  columns;       
  };

  explicit MailTable(QWidget *parent = nullptr);
  virtual ~MailTable() {};

  void initial(const MailTable::InitialSettings& settings, 
               const QList<MailboxModel::Columns>& defaultColumns);
  
private slots:
  /// \see ShowColumnAction::showColumn signal description.
  void onShowColumn(bool visible, MailboxModel::Columns column);
  /// Show default columns in the current mail box
  void onRestoreDefaultColumns();

private:
  void setupActions();
  /// Create action and add to horizontal header
  void initializeAction(MailboxModel::Columns columnType);
  /// Returns action name connected with column.
  /// This method is called when column name empty.
  QString getDefaultActionName(MailboxModel::Columns columnType);

private:
  QList<MailboxModel::Columns>  _defaultColumns;  
  ShowColumnAction* _columnActionArray[MailboxModel::NumColumns];
};