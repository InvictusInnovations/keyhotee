#include "MailTable.hpp"

#include <QAction>
#include <QHeaderView>
#include <QSortFilterProxyModel>

MailTable::MailTable(QWidget* parent)
  : QTableView(parent)
{
}

void MailTable::initial(const MailTable::InitialSettings& settings, 
                        const QList<MailboxModel::Columns>& defaultColumns)
{
  _defaultColumns = defaultColumns;

  horizontalHeader()->setContextMenuPolicy(Qt::ActionsContextMenu);  
  setShowGrid(false);
  verticalHeader()->setDefaultSectionSize(20);

  QHeaderView *horHeader = horizontalHeader();

  horHeader->setSectionsMovable(true);
  horHeader->setSortIndicatorShown(true);
  horHeader->setSectionsClickable(true);
  horHeader->setHighlightSections(true);

  horHeader->resizeSection(MailboxModel::To, 120);
  horHeader->resizeSection(MailboxModel::Subject, 300);
  horHeader->resizeSection(MailboxModel::DateReceived, 140);
  horHeader->resizeSection(MailboxModel::From, 120);
  horHeader->resizeSection(MailboxModel::DateSent, 140);

  for (uint i = 0; i < MailboxModel::NumColumns; ++i)
  {
    horHeader->hideSection (i);
  }
  const QList<MailboxModel::Columns>* columns;
  if (settings.columns.isEmpty())
  {
    columns = &_defaultColumns;
  }
  else
  {
    columns = &settings.columns;
  }
  for (const auto column : *columns)
  {
    horHeader->showSection (column);
  }

  sortByColumn(settings.sortColumn, settings.sortOrder);

  setupActions();
}

void MailTable::setupActions()
{
  for (uint i = 0; i < MailboxModel::NumColumns; ++i)
  {
    initializeAction ( static_cast<MailboxModel::Columns>(i) );
  }

  QAction* sep = new QAction(this);
  sep->setSeparator(true);
  QAction* restoreDefaultColumns = new QAction(tr("Restore default"), this);
  
  horizontalHeader()->addAction(sep);
  horizontalHeader()->addAction(restoreDefaultColumns);
  connect(restoreDefaultColumns, &QAction::triggered, 
          this, &MailTable::onRestoreDefaultColumns);
}

void MailTable::initializeAction(MailboxModel::Columns columnType)
{
  const QAbstractItemModel *model = this->model();  
  QString columnName = model->headerData (columnType, Qt::Horizontal,
                                          Qt::DisplayRole).toString();
  if (columnName.isEmpty())
  {
    // when column name is empty call getDefaultActionName.
    // Case for the column e.g. "Read", "Money"
    columnName = getDefaultActionName(columnType);
  }
  
  ShowColumnAction *action = 
                  new ShowColumnAction(columnName, this, columnType, 
                                       isColumnHidden(columnType) == false);
  // Do not let hide the column Subject.
  // That although one column was visible.
  if (columnType == MailboxModel::Subject)
  {
    action->setVisible(false);
  }

  _columnActionArray[columnType] = action;
  
  connect(action, SIGNAL( showColumn(bool, MailboxModel::Columns) ), 
          this, SLOT( onShowColumn(bool, MailboxModel::Columns) ));

  //add actions to horizontal header
  horizontalHeader()->addAction(action);
}

QString MailTable::getDefaultActionName(MailboxModel::Columns columnType)
{
  switch(columnType)
  {
  case MailboxModel::Read: return tr("Read");
  case MailboxModel::Money: return tr("Money");
  case MailboxModel::Attachment: return tr("Attachment");
  case MailboxModel::Reply: return tr("Reply");
  case MailboxModel::Chat: return tr("Chat");
  case MailboxModel::From: return tr("From");
  case MailboxModel::Subject: return tr("Subject");
  case MailboxModel::DateReceived: return tr("DateReceived");
  case MailboxModel::DateSent: return tr("DateSent");
  case MailboxModel::Status: return tr("Status");
  default:
    assert(0);
    return tr("not defined");
  }
}

void MailTable::onShowColumn(bool visible, MailboxModel::Columns column)
{
  setColumnHidden(column, visible == false);
}

void MailTable::onRestoreDefaultColumns()
{
  QHeaderView *horHeader = horizontalHeader();

  for (uint i = 0; i < MailboxModel::NumColumns; ++i)
  {
    horHeader->hideSection (i);
    _columnActionArray[i]->setChecked(false);
  }

  for (const auto column : _defaultColumns)
  {
    horHeader->showSection (column);
    _columnActionArray[column]->setChecked(true);
  }  
}