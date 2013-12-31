#include "MailboxModel.hpp"
#include "MessageHeader.hpp"

#include "utils.hpp"

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
    bool                         _isDraftFolder;
  };
}

MailboxModel::MailboxModel(QObject* parent, const bts::profile_ptr& profile,
  bts::bitchat::message_db_ptr mail_db, AddressBookModel& abModel, bool isDraftFolder)
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
    int           newRow = my->_headers.size();
    beginInsertRows(QModelIndex(), newRow, newRow);
    my->_headers.push_back(mail_header);
    endInsertRows();
    }
  }

void MailboxModel::replaceMessage(const TStoredMailMessage& overwrittenMsg,
                                  const TStoredMailMessage& msg)
  {
  for(auto& hdr : my->_headers)
    {
    if(hdr.header.digest == overwrittenMsg.digest)
      {
      MessageHeader helper;
      if(fillMailHeader(msg, helper))
        hdr = helper;

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
  my->_headers.reserve(headers.size());
  for (uint32_t i = 0; i < headers.size(); ++i)
    {
    MessageHeader helper;
    if(fillMailHeader(headers[i], helper))
      my->_headers.push_back(helper);
    }
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
    my->_mail_db->remove_message(my->_headers[i].header);
  //delete headers from in-memory my->_headers list
  auto rowI = my->_headers.begin() + row;
  my->_headers.erase(rowI, rowI + count);
  endRemoveRows();
  return true;
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
  MessageHeader& header = my->_headers[index.row()];
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
      switch (column)
        {
        case Read:
          return header.header.read_mark;
        case Money:
          return header.money_amount;
        case Attachment:
          return header.hasAttachments;
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
      if (!header.header.read_mark)
        {
        QFont boldFont;
        boldFont.setBold(true);
        return boldFont;
        }
      else
        return QVariant();
    } //switch (role)
  return QVariant();
  }

void MailboxModel::getFullMessage(const QModelIndex& index, MessageHeader& header) const
  {
  try
    {
    header = my->_headers[index.row()];
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
  MessageHeader& msg = my->_headers[index.row()];
  msg.header.read_mark = true;
  my->_mail_db->store_message_header(msg.header);
  }

QModelIndex MailboxModel::findModelIndex(const TStoredMailMessage& msg) const
  {
  int row = 0;
  for(const auto& hdr : my->_headers)
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

bool MailboxModel::hasAttachments(const QModelIndex& index) const
  {
  MessageHeader& msg = my->_headers[index.row()];
  return msg.hasAttachments;
  }
