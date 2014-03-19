#include "ShowColumnAction.hpp"

ShowColumnAction::ShowColumnAction(const QString& name, QObject* parent, 
                                    MailboxModel::Columns column, bool checked)
  :QAction(name, parent), 
  _column(column)
{  
  setCheckable(true);

  connect(this, SIGNAL(triggered(bool)), this, SLOT(onToggled(bool)));  
  setChecked(checked);  
}  


void ShowColumnAction::onToggled(bool checked)
{  
  emit showColumn(checked, _column);
}