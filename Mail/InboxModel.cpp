#include "InboxModel.hpp"
#include "MessageHeader.hpp"
#include <QIcon>
#include <QPixmap>
#include <QImage>

#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <bts/bitchat/bitchat_message_db.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/raw.hpp>

using namespace bts::bitchat;

namespace Detail 
{
    class InboxModelImpl
    {
       public:
          bts::profile_ptr              _profile;
          bts::bitchat::message_db_ptr  _mail_db;
          std::vector<MessageHeader>    _headers;
          QIcon                         _attachment_icon;
          QIcon                         _chat_icon;
          QIcon                         _read_icon;
          QIcon                         _money_icon;
    };
}

QDateTime toQDateTime( const fc::time_point_sec& time_in_seconds )
{
   QDateTime date_time;
   date_time.setTime_t(time_in_seconds.sec_since_epoch());
   return date_time;
}

InboxModel::InboxModel( QObject* parent, const bts::profile_ptr& profile, bts::bitchat::message_db_ptr mail_db)
: QAbstractTableModel(parent),
  my( new Detail::InboxModelImpl() )
{
   my->_profile = profile;
   my->_mail_db = mail_db;
   my->_attachment_icon = QIcon( ":/images/paperclip-icon.png" );
   my->_chat_icon = QIcon( ":/images/chat.png" );
   my->_money_icon = QIcon( ":/images/bitcoin.png" );
   my->_read_icon = QIcon( ":/images/read-icon.png" );

   readMailBoxHeadersDb(mail_db);
}

InboxModel::~InboxModel()
{
}

void InboxModel::fillMailHeader(const bts::bitchat::message_header& header,
                                MessageHeader& mail_header)
{
   auto addressbook = my->_profile->get_addressbook();
   mail_header.digest = header.digest;
   mail_header.date_received   = toQDateTime( header.received_time );
   auto to_contact   = addressbook->get_contact_by_public_key( header.to_key );
   auto from_contact = addressbook->get_contact_by_public_key( header.from_key );
   if( to_contact )
         mail_header.to   =  to_contact->dac_id_string.c_str();
   if( from_contact )
         mail_header.from =  from_contact->dac_id_string.c_str();
   mail_header.date_sent = toQDateTime( header.from_sig_time );
   mail_header.read_mark = header.read_mark;

//   mailHeader.subject = "Encrypted subject";
   auto raw_data = my->_mail_db->fetch_data(mail_header.digest);
   auto email_msg = fc::raw::unpack<private_email_message>(raw_data);
   mail_header.subject = email_msg.subject.c_str();
}

void InboxModel::addMailHeader(const bts::bitchat::message_header& header)
{
   MessageHeader mail_header;
   fillMailHeader(header, mail_header);
   int newRow = my->_headers.size();
   beginInsertRows(QModelIndex(),newRow,newRow);
   my->_headers.push_back(mail_header);
   endInsertRows();
}

void InboxModel::readMailBoxHeadersDb(bts::bitchat::message_db_ptr mail_db )
{
   auto headers = mail_db->fetch_headers(bts::bitchat::private_email_message::type );
   my->_headers.resize(headers.size());
   for( uint32_t i = 0; i < headers.size(); ++i )
   {
      fillMailHeader(headers[i],my->_headers[i]);
   }
}


int InboxModel::rowCount( const QModelIndex& parent )const
{
    return my->_headers.size();
}

int InboxModel::columnCount( const QModelIndex& parent  )const
{
    return NumColumns;
}

bool InboxModel::removeRows( int row, int count, const QModelIndex& parent )
{
   return false;
}

QVariant InboxModel::headerData( int section, Qt::Orientation orientation, int role )const
{
    if( orientation == Qt::Horizontal )
    {
       switch( role )
       {
          case Qt::DecorationRole:
             switch( (Columns)section )
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
              switch( (Columns)section )
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
              switch( (Columns)section )
              {
                  case Read:
                  case Money:
                  case Attachment:
                  case Chat:
                      return QSize( 8, 8 );
                 case From:
                      return QSize( 120, 24 );
                 case Subject:
                      return QSize( 300, 24 );
                 case DateReceived:
                    return QSize( 120, 12 );
                 case To:
                    return QSize( 120, 12 );
                 case Status:
                    return QSize( 120, 12 );
                 default:
                    return QSize( 16, 16);
              }
       }
    }
    else
    {
    }
    return QVariant();
}

QVariant InboxModel::data( const QModelIndex& index, int role )const
{
    if( !index.isValid() ) return QVariant();
    MessageHeader& header = my->_headers[index.row()];
  //  const & current_contact = my->_contacts[index.row()];
    switch( role )
    {
       case Qt::SizeHintRole:
           switch( (Columns)index.column() )
           {
               default:
                   return QVariant();
           }
       case Qt::DecorationRole:
          switch( (Columns)index.column() )
          {
            case Read:
            if (header.read_mark)
               return my->_read_icon;
            else
               return "";
//            case Money:
//               return QVariant();
            case Attachment:
               if (header.attachment)
                  return my->_attachment_icon;
               else
                  return "";
             default:
                return QVariant();
          }
       case Qt::DisplayRole:
          switch( (Columns)index.column() )
          {
             case Read:
                return header.read_mark;
             case Money:
                return header.money_amount;
             case Attachment:
                return header.attachment;
//             case Chat:
             case From:
                return header.from;
             case Subject:
                return header.subject;
             case DateReceived:
                return header.date_received;
             case To:
                return header.to;
             case DateSent:
                return header.date_sent;
             case Status:
                return QVariant(); //DLNFIX what is this?
             case NumColumns:
                return QVariant(); //DLNFIX what is this?
          }
    }
    return QVariant();
}

void InboxModel::getFullMessage( const QModelIndex& index, MessageHeader& header )const
{
   header = my->_headers[index.row()];
   auto raw_data = my->_mail_db->fetch_data(header.digest);
   auto email_msg = fc::raw::unpack<private_email_message>(raw_data);
   header.subject = email_msg.subject.c_str();
   header.body    = email_msg.body.c_str();
}