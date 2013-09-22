#include "InboxModel.hpp"
#include <QIcon>
#include <QPixmap>
#include <QImage>

#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <bts/bitchat/bitchat_message_db.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/raw.hpp>

namespace Detail 
{
    class InboxModelImpl
    {
       public:
          bts::profile_ptr              _user_profile;
          std::vector<MessageHeader>    _headers;
          QIcon                         _attachment_icon;
          QIcon                         _chat_icon;
          QIcon                         _read_icon;
          QIcon                         _money_icon;
    };
}

QDateTime toQDateTime( const fc::time_point_sec& sec )
{
   return QDateTime();
}

InboxModel::InboxModel( QObject* parent, const bts::profile_ptr& user_profile )
:QAbstractTableModel(parent),my( new Detail::InboxModelImpl() )
{
   my->_user_profile = user_profile;
   my->_attachment_icon = QIcon( ":/images/paperclip-icon.png" );
   my->_chat_icon = QIcon( ":/images/chat.png" );
   my->_money_icon = QIcon( ":/images/bitcoin.png" );
   my->_read_icon = QIcon( ":/images/read-icon.png" );

   std::vector<bts::bitchat::message_header>  headers = user_profile->get_inbox()->fetch_headers( 
                                                                                     bts::bitchat::private_email_message::type );
   my->_headers.resize(headers.size());

   auto abook = user_profile->get_addressbook();
   for( uint32_t i = 0; i < headers.size(); ++i )
   {
      my->_headers[i].digest          = headers[i].digest;
      my->_headers[i].date_received   = toQDateTime( headers[i].received_time );
      auto to_contact                 = abook->get_contact_by_public_key( headers[i].to_key );
      auto from_contact               = abook->get_contact_by_public_key( headers[i].from_key );
      if( to_contact )
      {
          my->_headers[i].to   =  to_contact->dac_id_string.c_str();
      }

      if( from_contact )
      {
          my->_headers[i].from =  from_contact->dac_id_string.c_str();
      }
//      my->_headers[i].date_sent     = toQDateTime( headers[i].
      my->_headers[i].read_mark       = headers[i].read_mark;
   }
}

InboxModel::~InboxModel()
{
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
                      return QSize( 12, 12 );
                 case From:
                      return QSize( 120, 12 );
                 case Subject:
                      return QSize( 300, 12 );
                 case DateReceived:
                    return QSize( 120, 12 );
                 case To:
                    return QSize( 120, 12 );
                  default:
                      return QVariant();
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
             default:
                return QVariant();
          }
       case Qt::DisplayRole:
          switch( (Columns)index.column() )
          {
             case Read:
             case Money:
             case Attachment:
             case Chat:
             case From:
             case Subject:
             case DateReceived:
             case To:
             case DateSent:
             case Status:
             case NumColumns:
                return QVariant();
          }
    }
    return QVariant();
}


bts::bitchat::decrypted_message InboxModel::getDecryptedMessage( const QModelIndex& index )const
{
   bts::bitchat::decrypted_message msg;
  return msg;
}
