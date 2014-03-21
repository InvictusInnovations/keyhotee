#pragma once

#include <QWidget>

namespace Ui { class Mailbox; }

#include "ch/mailprocessor.hpp"

class ATopLevelWindowsContainer;
class MailboxModel;

class QAbstractItemModel;
class QItemSelection;
class QSortFilterProxyModel;
class QTextEdit;

class Mailbox : public QWidget
{
  Q_OBJECT
public:
  enum InboxType
    {
    Inbox,
    Drafts,
    Outbox,
    Sent
    };

  Mailbox(ATopLevelWindowsContainer* parent = nullptr);
  virtual ~Mailbox();

  // void setModel(IMailProcessor& mailProcessor, MailboxModel* model, InboxType type = Inbox);
  void initial(IMailProcessor& mailProcessor, MailboxModel* model, InboxType type, ATopLevelWindowsContainer* parentKehoteeMainW);
  void searchEditChanged(QString search_string);

  bool isShowDetailsHidden();
  /// Allows to explicitly reread currently displayed message in the preview pane.
  void refreshMessageViewer();
  void removeMessage(const IMailProcessor::TStoredMailMessage& msg);
  bool isAttachmentSelected () const;
  void saveAttachment ();
  bool isSelection () const;
  bool isOneEmailSelected() const;
  void selectAll ();
  void checksendmailbuttons();
  /** Write mail box settings to the system registry
      when application is closing
  */
  void writeSettings();

public slots:
  void onReplyMail()
    {
    duplicateMail(ReplyType::reply);
    }

  void onReplyAllMail()
    {
    duplicateMail(ReplyType::reply_all);
    }

  void onForwardMail()
    {
    duplicateMail(ReplyType::forward);
    }

public slots:
  void onDeleteMail();
  void on_actionShow_details_toggled(bool checked);

private slots:
  void onDoubleClickedItem(QModelIndex);

private:
  struct Settings
  {
    int           sortColumn;
    Qt::SortOrder sortOrder;
  };

  enum ReplyType { reply, reply_all, forward };
  void setupActions();
  QModelIndex getSelectedMail();
  void showCurrentMail(const QModelIndex &selected, const QModelIndex &deselected);
  void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void selectNextRow(int idx, int deletedRowCount) const;
  void duplicateMail(ReplyType);
  bool getSelectedMessageData (IMailProcessor::TStoredMailMessage* encodedMsg,
                             IMailProcessor::TPhysicalMailMessage* decodedMsg);
  /// Read mail box settings from the system registry
  void readSettings();

private:
  QSortFilterProxyModel* sortedModel();
  /// Don't change 'ui' declaration since it breaks QTCreator tools 
  Ui::Mailbox*                 ui;
  InboxType                    _type;
  MailboxModel*                _sourceModel;
  IMailProcessor*              _mailProcessor;
  ATopLevelWindowsContainer*   _mainWindow;
  QAction*                     reply_mail;
  QAction*                     reply_all_mail;
  QAction*                     forward_mail;
  QAction*                     delete_mail;
  bool                         _attachmentSelected;
  Settings                     _settings;
};
