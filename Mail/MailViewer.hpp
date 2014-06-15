#include <QWidget>
#include <memory>
#include <bts/bitchat/bitchat_private_message.hpp>
#include "MessageHeader.hpp"

namespace Ui { class MailViewer; }
class QToolBar;
class MailboxModel;
class Mailbox;

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
  void displayMailMessage(Mailbox*, const QModelIndex& index, MailboxModel* mailbox);

private slots:
  /// Display blocked remote images
  void onShowRemoteContent();

public:
  QToolBar* message_tools;
private:
  Ui::MailViewer* ui;
};
