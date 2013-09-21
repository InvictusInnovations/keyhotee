#include "InboxModel.hpp"
#include <QIcon>
#include <QPixmap>
#include <QImage>

#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/raw.hpp>

namespace Detail 
{
    class InboxModelImpl
    {
       public:
          bts::bitchat::message_db_ptr  _msg_db;
          std::vector<MessageHeader>    _headers;
    };
}

InboxModel::InboxModel( QObject* parent, const bts::bitchat::message_db_ptr& msg_db )
:QAbstractTableModel(parent),my( new Detail::InboxModelImpl() )
{
   my->_msg_db = msg_db;
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
          //      case UserIcon:
          //          return my->_default_icon;
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
