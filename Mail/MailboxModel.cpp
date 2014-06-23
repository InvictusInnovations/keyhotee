#include "MailboxModel.hpp"
#include "MessageHeader.hpp"

#include "utils.hpp"

#include <QIcon>
#include <QPixmap>
#include <QImage>
#include <QTreeWidgetItem>

#include <bts/bitchat/bitchat_message_db.hpp>
#include <bts/address.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/raw.hpp>

using namespace bts::bitchat;

typedef fc::uint256                                       TDigest;
typedef std::map<TDigest, MessageHeader*>                 TDigest2MsgHeader;
typedef std::list<MessageHeader>                          TCacheStorage;
typedef TCacheStorage::iterator                           TCacheStorageItr;
typedef TDigest2MsgHeader::iterator                       TDigest2MsgHeaderItr;
typedef std::pair<TCacheStorageItr, TDigest2MsgHeaderItr> TCacheData;
typedef std::vector<TCacheData>                           TRandomAccessCache;

namespace Detail
{
class MailboxModelImpl
  {
  public:
    bts::profile_ptr             _profile;
    bts::bitchat::message_db_ptr _mail_db;
    /// FIXME - potential perf. problem while adding/removing message headers (while reallocating vector)
    //std::vector<MessageHeader>   _headers;
    
    TDigest2MsgHeader            _digest2headers;
    TCacheStorage                _headers_storage;
    TRandomAccessCache           _headers_random;

    QIcon                        _attachment_icon;
    QIcon                        _chat_icon;
    QIcon                        _read_icon;
    QIcon                        _money_icon;
    AddressBookModel*            _abModel;
    bool                         _isDraftFolder;
  };
}

MailboxModel::MailboxModel(QObject* parent, const bts::profile_ptr& profile,
  bts::bitchat::message_db_ptr mail_db, AddressBookModel& abModel, QTreeWidgetItem* tree_item, bool isDraftFolder)
  : QAbstractTableModel(parent),
  my(new Detail::MailboxModelImpl()),
  _unread_msg_count(0),
  _tree_item(tree_item),
  _item_name(tree_item->text(0))
  {
  my->_profile = profile;
  my->_mail_db = mail_db;
  my->_attachment_icon = QIcon(":/images/paperclip-icon.png");
  my->_chat_icon = QIcon(":/images/chat.png");
  my->_money_icon = QIcon(":/images/bitcoin.png");
  my->_read_icon = QIcon(":/images/read-icon.png");
  my->_abModel = &abModel;
  my->_isDraftFolder = isDraftFolder;

  readMailBoxHeadersDb(mail_db);
  }
   
MailboxModel::~MailboxModel()
  {}

bool MailboxModel::fillMailHeader(const bts::bitchat::message_header& header,
  MessageHeader& mail_header)
  {
  try
    {
    if (my->_digest2headers.find( header.digest ) != my->_digest2headers.end() )
      FC_THROW("Received a duplicate message, ignoring");
    mail_header.header = header;
    auto addressbook = my->_profile->get_addressbook();
    mail_header.date_received = Utils::toQDateTime(header.received_time);

    mail_header.from = Utils::toString(header.from_key, Utils::FULL_CONTACT_DETAILS);

    mail_header.date_sent = Utils::toQDateTime(header.from_sig_time);

    //fill remaining fields from private_email_message
    auto raw_data = my->_mail_db->fetch_data(header.digest);
    auto email_msg = fc::raw::unpack<private_email_message>(raw_data);

    mail_header.to_list = email_msg.to_list;
    mail_header.cc_list = email_msg.cc_list;
    mail_header.subject = email_msg.subject.c_str();
    mail_header.hasAttachments = email_msg.attachments.size();
    return true;
    }
  catch(const fc::exception& e)
    {
    mail_header = MessageHeader();
    elog("${e}", ("e", e.to_detail_string()));
    return false;
    }
  }


void MailboxModel::addMailHeader(const bts::bitchat::message_header& header)
{
  MessageHeader mail_header;
  if(fillMailHeader(header, mail_header))
  {
    assert(my->_headers_random.size() == my->_headers_storage.size());
    int newRow = my->_headers_random.size();
    beginInsertRows(QModelIndex(), newRow, newRow);

    pushBack(mail_header);

    endInsertRows();

    if(mail_header.header.isUnread())
      _unread_msg_count++;

    updateTreeItemDisplay();
  }

}

void MailboxModel::pushBack(const MessageHeader& mail_header)
{
  TCacheData data;
  data.first = my->_headers_storage.insert(my->_headers_storage.end(), mail_header);
  MessageHeader& storedHeader = *data.first;
  auto result = my->_digest2headers.insert(TDigest2MsgHeader::value_type(storedHeader.header.digest, &storedHeader));
  assert(result.second); //check to be sure no duplicate hash in mailbox
  data.second = result.first;
  my->_headers_random.push_back(data);
}

void MailboxModel::replaceMessage(const TStoredMailMessage& overwrittenMsg,
                                  const TStoredMailMessage& msg)
  {
  for(auto& hdr : my->_headers_storage)                 // ****************************************************************************
    {
    if(hdr.header.digest == overwrittenMsg.digest)
      {
      MessageHeader helper;
      if(fillMailHeader(msg, helper))
      {
        hdr = helper;

        if(overwrittenMsg.isRead())
          _unread_msg_count++;
        updateTreeItemDisplay();
      }

      /// Replace complete - return.
      return;
      }
    }

  /** At this point new message entry must be added since old one was not found (maybe deleted while
      editing)
  */
  addMailHeader(msg);
  }

void MailboxModel::readMailBoxHeadersDb(bts::bitchat::message_db_ptr mail_db)
{
  auto headers = mail_db->fetch_headers(bts::bitchat::private_email_message::type);
  for (uint32_t i = 0; i < headers.size(); ++i)
  {
    MessageHeader helper;
    if(fillMailHeader(headers[i], helper))
    {
      pushBack(helper);
      if(helper.header.isUnread())
        _unread_msg_count++;
    }
  }
  updateTreeItemDisplay();
}

int MailboxModel::rowCount(const QModelIndex& parent) const
  {
    return my->_headers_random.size();
  }

int MailboxModel::columnCount(const QModelIndex& parent) const
  {
  return NumColumns;
  }

bool MailboxModel::removeRows(int row, int count, const QModelIndex&)
  {
  beginRemoveRows(QModelIndex(), row, row + count - 1);
  for (int i = row; i < row + count; ++i)
    removeRow(i);
  endRemoveRows();
  updateTreeItemDisplay();
  return true;

  }

void MailboxModel::removeRow(int row_index)
{
  auto data = my->_headers_random[row_index];
  if((*data.first).header.isUnread())
    _unread_msg_count--;
  my->_mail_db->remove_message((*data.first).header);
  my->_headers_storage.erase(data.first);
  my->_digest2headers.erase(data.second);
  auto rowI = my->_headers_random.begin() + row_index;
  my->_headers_random.erase(rowI);
}

QVariant MailboxModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
  if (orientation == Qt::Horizontal)
    {
    Columns column = (Columns)section;
    switch (role)
      {
      case Qt::DecorationRole:
        switch (column)
          {
          case Read:
            return my->_read_icon;
          case Money:
            return my->_money_icon;
          case Attachment:
            return my->_attachment_icon;
          case Chat:
            return my->_chat_icon;
          default:
            return QVariant();
          }
      case Qt::DisplayRole:
        switch (column)
          {
          case Read:
          case Money:
          case Attachment:
          case Reply:
          case Chat:
            return QVariant();
          case From:
            return tr("From");
          case Subject:
            return tr("Subject");
          case DateReceived:
            return tr("Date Received");
          case To:
            return tr("To");
          case DateSent:
            return my->_isDraftFolder ? tr("Date Saved") : tr("Date Sent");
          case Status:
            return tr("Status");
          }
      case Qt::TextAlignmentRole:
        switch (column)
          {
          case From:
          case Subject:
          case DateReceived:
          case To:
          case DateSent:
          case Status:
            return Qt::AlignLeft + Qt::AlignVCenter;
          default:
            return QVariant();
          } //switch columns in TextAlignmentRole
      case Qt::SizeHintRole:
        switch (column)
          {
          case Read:
          case Money:
          case Attachment:
          case Reply:
          case Chat:
            return QSize(8, 8);
          case From:
            return QSize(120, 24);
          case Subject:
            return QSize(300, 24);
          case DateReceived:
            return QSize(120, 12);
          case To:
            return QSize(120, 12);
          case Status:
            return QSize(120, 12);
          default:
            return QSize(16, 16);
          } //switch columns in SizeHintRole
        } //switch role
    } //if (orientation == Qt::Horizontal)
  return QVariant();
  }

QVariant MailboxModel::data(const QModelIndex& index, int role) const
  {
  if (!index.isValid() )
    return QVariant();

  auto storage_itr = my->_headers_random[index.row()].first;
  MessageHeader& header = *storage_itr;

  Columns column = (Columns)index.column();
  switch (role)
    {
    case Qt::SizeHintRole:
      switch (column)
        {
        default:
          return QVariant();
        } //switch (column)
    case Qt::DecorationRole:
      switch (column)
        {
        case Read:
          if (header.header.isUnread())
            return my->_read_icon;
          else
            return "";
        //            case Money:
        //               return QVariant();
        case Attachment:
          if (header.hasAttachments)
            return my->_attachment_icon;
          else
            return "";
        case Reply:
          if(header.header.isReplied() && header.header.isForwarded())
            return QIcon(":/images/mail_replied_forwarded.png");
          else if(header.header.isReplied())
            return QIcon(":/images/mail_replied.png");
          else if(header.header.isForwarded())
            return QIcon(":/images/mail_forwarded.png");
        default:
          return QVariant();
        }
    case Qt::DisplayRole:
      switch (column)
        {
        case Read:
          return header.header.isRead();
        case Money:
          return header.money_amount;
        case Attachment:
          return header.hasAttachments;
        case Reply:
          return QVariant();
        //             case Chat:
        case From:
          return Utils::toString(header.header.from_key, Utils::FULL_CONTACT_DETAILS);
        case Subject:
          return header.subject;
        case DateReceived:
          return header.date_received;
        case To:
          return Utils::makeContactListString(header.to_list, ';', Utils::FULL_CONTACT_DETAILS);
        case DateSent:
          return header.date_sent;
        case Status:
          return QVariant();           //DLNFIX what is this?
        } //switch (column)
    case Qt::FontRole:
      if (header.header.isUnread())
      {
        QFont boldFont;
        boldFont.setBold(true);
        return boldFont;
      }
      else
        return QVariant();
    case Qt::ToolTipRole:
      switch (column)
      {
      case Reply:
        if(header.header.isReplied() && header.header.isForwarded())
          return tr("Replied and Forwarded");
        else if(header.header.isReplied())
          return tr("Replied");
        else if(header.header.isForwarded())
          return tr("Forwarded");
      default:
        return QVariant();
      } //switch column in ToolTipRole
    } //switch (role)
  return QVariant();
  }

void MailboxModel::getFullMessage(const QModelIndex& index, MessageHeader& header) const
  {
  try
    {
    header = *(my->_headers_random[index.row()].first);
    /// Update sender info each time to match data defined in contact/identity list.
    header.from = Utils::toString(header.header.from_key, Utils::TContactTextFormatting::FULL_CONTACT_DETAILS);

    auto raw_data = my->_mail_db->fetch_data(header.header.digest);
    auto email_msg = fc::raw::unpack<private_email_message>(raw_data);

    header.to_list = email_msg.to_list;
    header.cc_list = email_msg.cc_list;
    header.subject = email_msg.subject.c_str();
    header.body = email_msg.body.c_str();
    header.attachments = email_msg.attachments;
    }
  catch(const fc::exception& e)
    {
    elog("${e}", ("e", e.to_detail_string()));
    header = MessageHeader();
    }
  }

void MailboxModel::markMessageAsRead(const QModelIndex& index)
  {
  MessageHeader& msg = *(my->_headers_random[index.row()].first);
  if(msg.header.isRead())
    return;
  msg.header.setRead();

  _unread_msg_count--;
  updateTreeItemDisplay();

  my->_mail_db->store_message_header(msg.header);
  }

void MailboxModel::markMessageAsUnread(const QModelIndex& index)
{
  MessageHeader& msg = *(my->_headers_random[index.row()].first);
  if (msg.header.isUnread())
    return;
  msg.header.setUnread();

  _unread_msg_count++;
  updateTreeItemDisplay();

  my->_mail_db->store_message_header(msg.header);
}

void MailboxModel::markMessageAsReplied(const QModelIndex& index)
{
  MessageHeader& msg = *(my->_headers_random[index.row()].first);
  if(msg.header.isReplied())
    return;
  msg.header.setReplied();
  my->_mail_db->store_message_header(msg.header);
  
  QModelIndex start = createIndex(index.row(), Reply);
  QModelIndex stop = createIndex(index.row(), Reply);
  emit dataChanged(start, stop);
}

void MailboxModel::markMessageAsForwarded(const QModelIndex& index)
{
  MessageHeader& msg = *(my->_headers_random[index.row()].first);
  if(msg.header.isForwarded())
    return;
  msg.header.setForwarded();
  my->_mail_db->store_message_header(msg.header);

  QModelIndex start = createIndex(index.row(), Reply);
  QModelIndex stop = createIndex(index.row(), Reply);
  emit dataChanged(start, stop);
}

QModelIndex MailboxModel::findModelIndex(const TStoredMailMessage& msg) const
  {
  int row = 0;
  for(const auto& hdr : my->_headers_storage)                 // ****************************************************************************
    {
    if(hdr.header.digest == msg.digest)
      {
      /// Replace complete - return.
      return index(row, 0);
      }
    ++row;
    }

  return QModelIndex();
  }

QModelIndex MailboxModel::findModelIndex(const fc::uint256& digest) const
{
  try
  {
    MessageHeader* message_header = my->_digest2headers.at(digest);
    return findModelIndex(message_header->header);
  }
  catch (const std::out_of_range& e)
  {
    elog("${e}", ("e", e.what()));
    return QModelIndex();
  }
}

void MailboxModel::getMessageData(const QModelIndex& index,
  IMailProcessor::TStoredMailMessage* encodedMsg, IMailProcessor::TPhysicalMailMessage* decodedMsg)
  {
  const MessageHeader& cachedMsg = *(my->_headers_random[index.row()].first);
  *encodedMsg = cachedMsg.header;

  try
    {
      auto raw_data = my->_mail_db->fetch_data(cachedMsg.header.digest);
      *decodedMsg = fc::raw::unpack<private_email_message>(raw_data);
    }
  catch(const fc::exception& e)
    {
    elog("${e}", ("e", e.to_detail_string()));
    *decodedMsg = IMailProcessor::TPhysicalMailMessage();
    }
  }

AddressBookModel& MailboxModel::getAddressBookModel() const
  {
  return *my->_abModel;
  }

bool MailboxModel::hasAttachments(const QModelIndex& index) const
  {
  MessageHeader& msg = *(my->_headers_random[index.row()].first);
  return msg.hasAttachments;
  }

void MailboxModel::updateTreeItemDisplay()
{
  QString display_text;
  QFont font;
  if (_unread_msg_count)
  {
    display_text = QString("%1 (%2)").arg(_item_name).arg(_unread_msg_count);
    font.setBold(true);
  }
  else
  {
    display_text = _item_name;
    font.setBold(false);
  }
  _tree_item->setFont(0, font);
  _tree_item->setText(0, display_text);

  QString tool_tip(tr("Unread %1 / All %2").arg(_unread_msg_count).arg(my->_headers_random.size()));
  _tree_item->setToolTip(0, tool_tip);
}
