#pragma once

#include <QList>
#include <QWidget>

namespace Ui { class Mailbox; }

#include "ch/mailprocessor.hpp"

#include "MailboxModel.hpp"
#include "MailTable.hpp"

class ATopLevelWindowsContainer;

class QAbstractItemModel;
class QItemSelection;
class QSortFilterProxyModel;
class QTextEdit;

class Mailbox : public QWidget
{
  Q_OBJECT
  /// Register enums for name use at runtime
  Q_ENUMS(InboxType)
public:
  enum InboxType
    {
    Inbox,
    Drafts,
    Outbox,
    Sent,
    Spam
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
  /** Disable/enable actionReply, actionReply_All, actionForward buttons
      depending on one email is selected and identity exist
  */
  void checkSendMailButtons();
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

  void onOpenMail();
  void onMarkAsUnreadMail();

private:
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
  void readSettings(MailTable::InitialSettings* initSettings);

  /** Each column encode to one bit: 1 << columnType
      Return shift bit array of selected columns
  */
  unsigned int encodeSelectedColumns();

  /// Decode columns from bit array to list of columns
  void decodeSelectedColumns(unsigned int columnsEncode/*in*/, 
                             QList<MailboxModel::Columns>* columnsDecode/*out*/);

  void getDefaultColumns(QList<MailboxModel::Columns>* defaultColumns);

  bool isIdentity();

private:
  QSortFilterProxyModel* sortedModel();
  /// Don't change 'ui' declaration since it breaks QTCreator tools 
  Ui::Mailbox*                 ui;
  InboxType                    _type;
  MailboxModel*                _sourceModel;
  IMailProcessor*              _mailProcessor;
  ATopLevelWindowsContainer*   _mainWindow;
  //QAction*                     reply_mail;
  //QAction*                     reply_all_mail;
  //QAction*                     forward_mail;
  //QAction*                     delete_mail;
  bool                         _attachmentSelected;
};
