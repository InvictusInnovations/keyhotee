#include "MailTable.hpp"

#include <QAction>
#include <QHeaderView>

MailTable::MailTable(QWidget* parent)
  : QTableView(parent)
{
}

void MailTable::initialActions()
{
  horizontalHeader()->setContextMenuPolicy(Qt::ActionsContextMenu);
  setupActions();
}

void MailTable::setupActions()
{
  initializeAction( tr("Read"), MailboxModel::Read);
  initializeAction( tr("Money"), MailboxModel::Money);
  initializeAction( tr("Attachment"), MailboxModel::Attachment);
  initializeAction( tr("Reply"), MailboxModel::Reply);
  initializeAction( tr("Chat"), MailboxModel::Chat);
  initializeAction( tr("From"), MailboxModel::From);
  initializeAction( tr("To"), MailboxModel::To);
  initializeAction( tr("Date Received"), MailboxModel::DateReceived);  
  initializeAction( tr("Date Sent"), MailboxModel::DateSent);
  initializeAction( tr("Status"), MailboxModel::Status);  
}

void MailTable::initializeAction(QString columnName, MailboxModel::Columns columnType)
{
  ShowColumnAction* action = new ShowColumnAction(columnName, this, columnType, isColumnHidden(columnType) == false);
  
  connect(action,  SIGNAL( showColumn(bool, MailboxModel::Columns) ), 
        this, SLOT( onShowColumn(bool, MailboxModel::Columns) ));

  //add actions to horizontal header
  horizontalHeader()->addAction(action);
}

void MailTable::onShowColumn(bool visible, MailboxModel::Columns column)
{
  setColumnHidden(column, visible == false);
}