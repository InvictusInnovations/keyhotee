#pragma once
#include <QtGui>
#include <bts/profile.hpp>

#include "ch/mailprocessor.hpp"

class MessageHeader;

namespace Detail { class MailboxModelImpl; }

class AddressBookModel;
class QTreeWidgetItem;

class MailboxModel : public QAbstractTableModel
{
public:
  typedef IMailProcessor::TStoredMailMessage                TStoredMailMessage;

  /** Class constructor.
      \param abModel - access to the main address book model, needed for message editing purposes
  */
  MailboxModel(QObject* parent, const bts::profile_ptr& user_profile,
    bts::bitchat::message_db_ptr mail_db, AddressBookModel& abModel, QTreeWidgetItem* tree_item, bool isDraftFolder);
  virtual ~MailboxModel();

  enum Columns
    {
    Read,
    Money,
    Attachment,
    Reply,
    Chat,
    From,
    Subject,
    DateReceived,
    To,
    DateSent,
    Status,
    NumColumns
    };

  void addMailHeader(const bts::bitchat::message_header& header);
  /** Allows to replace model internal data structures with new message. Used for updating draft messages.
      \param overwrittenMsg - message to be replaced,
      \param msg - new message, to be stored & displayed by view associated with this model.
  */
  void replaceMessage(const TStoredMailMessage& overwrittenMsg, const TStoredMailMessage& msg);
  void getFullMessage(const QModelIndex& index, MessageHeader& header) const;
  void markMessageAsRead(const QModelIndex& index);
  void markMessageAsUnread(const QModelIndex& index);
  void markMessageAsReplied(const QModelIndex& index);
  void markMessageAsForwarded(const QModelIndex& index);
  QModelIndex findModelIndex(const TStoredMailMessage& msg) const;
  QModelIndex findModelIndex(const fc::uint256& digest) const;
  /** Allows to retrieve given message data in encoded & decoded from.
      Encoded form (the message_header) is needed to retrieve sender for example.
      Decoded message contains all others attributes.

      \param index - index to mail model row to retrieve data for,
      \param encodedMsg - output, will hold encoded message data (message_header),
      \param decodedMsg - output, will hold decoded message data (private_email_message)
  */
  void getMessageData(const QModelIndex& index, TStoredMailMessage* encodedMsg,
    IMailProcessor::TPhysicalMailMessage* decodedMsg);

  /// Gives access to the address book model associated to current model.
  AddressBookModel& getAddressBookModel() const;

  virtual int rowCount(const QModelIndex& parent = QModelIndex() ) const;
  virtual int columnCount(const QModelIndex& parent = QModelIndex() ) const;
  virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());

  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  bool hasAttachments(const QModelIndex& index) const;

private:
  bool fillMailHeader(const bts::bitchat::message_header& header, MessageHeader& mail_header);

  void readMailBoxHeadersDb(bts::bitchat::message_db_ptr mail_db);

  void pushBack(const MessageHeader& mail_header);
  void removeRow(int row_index);

  void updateTreeItemDisplay();

private:
  std::unique_ptr<Detail::MailboxModelImpl> my;

  QTreeWidgetItem*    _tree_item;
  unsigned int        _unread_msg_count;
  const QString       _item_name;
};
