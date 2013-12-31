#include <QWidget>
#include <memory>
#include <bts/bitchat/bitchat_private_message.hpp>
#include "MessageHeader.hpp"

namespace Ui { class MailViewer; }
class QToolBar;
class MailboxModel;

class MailViewer : public QWidget
{
  Q_OBJECT
public:
  MailViewer(QWidget* parent = nullptr);
  virtual ~MailViewer();
  
  /** Displays mail and marks as having been read
      \param index - model index referencing mail message to be displayed. Must be valid.
      \param mailbox - mailbox model, cannot be null.
  */
  void displayMailMessage(const QModelIndex& index, MailboxModel* mailbox);
  void displayMailMessages(QModelIndexList, QItemSelectionModel * mailbox);

  QToolBar* message_tools;
private:
  void displayAttachments(const MessageHeader& msg);

  std::unique_ptr<Ui::MailViewer> ui;
};
