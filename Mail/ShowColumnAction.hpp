#pragma once

#include "MailboxModel.hpp"

#include <QAction>

/** ShowColumnAction class emits signal showColumn when user right mouse button click
    on header of the table of mail.
*/
class ShowColumnAction : public QAction  
{  
    Q_OBJECT
public:  
  /**
     \param name - column action name
     \param parent - MailTable parent
     \param column - column type: "To", "From", etc.
     \param checked - initial state of check box
  */
  ShowColumnAction(const QString& name, QObject* parent, MailboxModel::Columns column, 
                   bool checked);

public slots:
  void onToggled(bool checked);

Q_SIGNALS:
  /** Signal
      \param checked -  true - show column table of mail
                        false - hide column table of mail
      \param column - column which should be hide/show
  */
  void showColumn(bool checked, MailboxModel::Columns column);

private:  
  MailboxModel::Columns _column;
};