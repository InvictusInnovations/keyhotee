#include "MailboxModel.hpp"
#include "MessageHeader.hpp"
#include "public_key_address.hpp"
#include <QIcon>
#include <QPixmap>
#include <QImage>

#include <bts/bitchat/bitchat_message_db.hpp>
#include <bts/address.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/raw.hpp>

using namespace bts::bitchat;

namespace Detail
{
class MailboxModelImpl
  {
  public:
    bts::profile_ptr             _profile;
    bts::bitchat::message_db_ptr _mail_db;
    /// FIXME - potential perf. problem while adding/removing message headers (while reallocating vector)
    std::vector<MessageHeader>   _headers;
    QIcon                        _attachment_icon;
    QIcon                        _chat_icon;
    QIcon                        _read_icon;
    QIcon                        _money_icon;
    AddressBookModel*            _abModel;
  };
}

QDateTime toQDateTime(const fc::time_point_sec& time_in_seconds)
  {
  QDateTime date_time;
  date_time.setTime_t(time_in_seconds.sec_since_epoch());
  return date_time;
  }

//DLNFIX move this to utility function file
QString makeContactListString(std::vector<fc::ecc::public_key> key_list)
  {
  QStringList to_list;
  QString     to;
  std::string public_key_string;
  auto        address_book = bts::get_profile()->get_addressbook();
  foreach(auto public_key, key_list)
    {
    auto contact = address_book->get_contact_by_public_key(public_key);
    if (contact)
      {
      to_list.append(contact->dac_id_string.c_str());
      }
    else   //display public_key as base58
      {
      std::string public_key_string = public_key_address(public_key);
      to_list.append(public_key_string.c_str());
      }
    }
  return to_list.join(',');
  }

MailboxModel::MailboxModel(QObject* parent, const bts::profile_ptr& profile,
  bts::bitchat::message_db_ptr mail_db, AddressBookModel& abModel)
  : QAbstractTableModel(parent),
  my(new Detail::MailboxModelImpl() )
  {
  my->_profile = profile;
  my->_mail_db = mail_db;
  my->_attachment_icon = QIcon(":/images/paperclip-icon.png");
  my->_chat_icon = QIcon(":/images/chat.png");
  my->_money_icon = QIcon(":/images/bitcoin.png");
  my->_read_icon = QIcon(":/images/read-icon.png");
  my->_abModel = &abModel;

  readMailBoxHeadersDb(mail_db);
  }

MailboxModel::~MailboxModel()
  {}

void MailboxModel::fillMailHeader(const bts::bitchat::message_header& header,
                                  MessageHeader& mail_header)
  {
  mail_header.header = header;
  auto addressbook = my->_profile->get_addressbook();
  mail_header.date_received = toQDateTime(header.received_time);

  // Later, we might want to do this in data function instead (as we do for to_list)
  // It would be slightly slower, but unknown keys would change to known contact names
  // when added to contact list
  auto from_contact = addressbook->get_contact_by_public_key(header.from_key);
  if (from_contact)
    mail_header.from = from_contact->dac_id_string.c_str();
  else
    mail_header.from = std::string(bts::address(header.from_key)).c_str();

  mail_header.date_sent = toQDateTime(header.from_sig_time);

  //fill remaining fields from private_email_message
  auto raw_data = my->_mail_db->fetch_data(header.digest);
  auto email_msg = fc::raw::unpack<private_email_message>(raw_data);
  mail_header.to_list = email_msg.to_list;
  mail_header.cc_list = email_msg.cc_list;
  mail_header.subject = email_msg.subject.c_str();
  mail_header.hasAttachments = email_msg.attachments.size();
  }

void MailboxModel::addMailHeader(const bts::bitchat::message_header& header)
  {
  MessageHeader mail_header;
  fillMailHeader(header, mail_header);
  int           newRow = my->_headers.size();
  beginInsertRows(QModelIndex(), newRow, newRow);
  my->_headers.push_back(mail_header);
  endInsertRows();
  }

void MailboxModel::replaceMessage(const TStoredMailMessage& overwrittenMsg,
  const TStoredMailMessage& msg)
  {
  for(auto& hdr : my->_headers)
    {
    if(hdr.header.digest == overwrittenMsg.digest)
      {
      hdr = MessageHeader();
      fillMailHeader(msg, hdr);

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
  my->_headers.resize(headers.size());
  for (uint32_t i = 0; i < headers.size(); ++i)
    fillMailHeader(headers[i], my->_headers[i]);
  }

int MailboxModel::rowCount(const QModelIndex& parent) const
  {
  return my->_headers.size();
  }

int MailboxModel::columnCount(const QModelIndex& parent) const
  {
  return NumColumns;
  }

bool MailboxModel::removeRows(int row, int count, const QModelIndex&)
  {
  beginRemoveRows(QModelIndex(), row, row + count - 1);
  for (int i = row; i < row + count; ++i)
    my->_mail_db->remove(my->_headers[i].header);
     //delete headers from my->_headers
  auto rowI = my->_headers.begin() + row;
  my->_headers.erase(rowI, rowI + count);
  endRemoveRows();
  return true;
  }

QVariant MailboxModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
  if (orientation == Qt::Horizontal)
    {
    switch (role)
      {
    case Qt::DecorationRole:
      switch ( (Columns)section)
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
        {
        switch ( (Columns)section)
          {
        case Read:
        case Money:
        case Attachment:
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
          return tr("Date Sent");
        case Status:
          return tr("Status");

        case NumColumns:
          break;
          }
        }
    case Qt::SizeHintRole:
      switch ( (Columns)section)
        {
      case Read:
      case Money:
      case Attachment:
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
        }
      }
    }
  else
            {}
  return QVariant();
  }

QVariant MailboxModel::data(const QModelIndex& index, int role) const
  {
  if (!index.isValid() )
    return QVariant();
  MessageHeader& header = my->_headers[index.row()];
  //  const & current_contact = my->_contacts[index.row()];
  switch (role)
    {
  case Qt::SizeHintRole:
    switch ( (Columns)index.column() )
      {
    default:
      return QVariant();
      }
  case Qt::DecorationRole:
    switch ( (Columns)index.column() )
      {
    case Read:
      if (!header.header.read_mark)
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
    default:
      return QVariant();
      }
  case Qt::DisplayRole:
    switch ( (Columns)index.column() )
      {
    case Read:
      return header.header.read_mark;
    case Money:
      return header.money_amount;
    case Attachment:
      return header.hasAttachments;
    //             case Chat:
    case From:
      return header.from;
    case Subject:
      return header.subject;
    case DateReceived:
      return header.date_received;
    case To:
        {
        return makeContactListString(header.to_list);
        }
    case DateSent:
      return header.date_sent;
    case Status:
      return QVariant();           //DLNFIX what is this?
    case NumColumns:
      return QVariant();           //DLNFIX what is this?
      }
    }
  return QVariant();
  }

void MailboxModel::getFullMessage(const QModelIndex& index, MessageHeader& header) const
  {
  header = my->_headers[index.row()];
  auto raw_data = my->_mail_db->fetch_data(header.header.digest);
  auto email_msg = fc::raw::unpack<private_email_message>(raw_data);
  header.to_list = email_msg.to_list;
  header.cc_list = email_msg.cc_list;
  header.subject = email_msg.subject.c_str();
  header.body = email_msg.body.c_str();
  header.attachments = email_msg.attachments;
  }

void MailboxModel::markMessageAsRead(const QModelIndex& index)
  {
  MessageHeader& msg = my->_headers[index.row()];
  msg.header.read_mark = true;
  my->_mail_db->store_message_header(msg.header);
  }

void MailboxModel::getMessageData(const QModelIndex& index,
  IMailProcessor::TStoredMailMessage* encodedMsg, IMailProcessor::TPhysicalMailMessage* decodedMsg)
  {
  const MessageHeader& cachedMsg = my->_headers[index.row()];
  *encodedMsg = cachedMsg.header;

  auto rawData = my->_mail_db->fetch_data(cachedMsg.header.digest);
  *decodedMsg = fc::raw::unpack<private_email_message>(rawData);
  }

AddressBookModel& MailboxModel::getAddressBookModel() const
  {
  return *my->_abModel;
  }

