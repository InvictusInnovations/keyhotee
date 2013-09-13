#include "AddressBookModel.hpp"
#include <QIcon>
#include <QPixmap>
#include <QImage>

#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/raw.hpp>


namespace Detail 
{
    class AddressBookModelImpl
    {
       public:
          QIcon                                   _default_icon;
          std::vector<Contact>                    _contacts;
          bts::addressbook::addressbook_ptr       _abook;

          Contact convert_contact( const bts::addressbook::contact& bts_contact )
          {
              Contact new_contact;
              if( bts_contact.icon_png.size() )
              {
                  QImage img;
                  if( img.loadFromData( (unsigned char*)bts_contact.icon_png.data(), bts_contact.icon_png.size() ) )
                  {
                    new_contact.icon = QIcon( QPixmap::fromImage(img) );
                  }
                  else
                  {
                      wlog( "unable to load icon for contact ${c}", ("c",bts_contact) );
                  }
              }
              new_contact.first_name           = bts_contact.first_name.c_str();
              new_contact.last_name            = bts_contact.last_name.c_str();
              new_contact.company              = bts_contact.company.c_str();
              new_contact.phone_number         = bts_contact.phone_number.c_str();
              new_contact.email_address        = bts_contact.email_address.c_str();
              new_contact.bit_id               = bts_contact.bit_id.c_str();
              new_contact.public_key           = bts_contact.send_msg_address;
              new_contact.known_since.setMSecsSinceEpoch( bts_contact.known_since.time_since_epoch().count()/1000 );
              new_contact.privacy_setting      = bts_contact.privacy_setting;
              new_contact.wallet_account_index = bts_contact.wallet_account_index;

              return new_contact;
          }
          bts::addressbook::contact convert_contact( const Contact& con )
          {
              bts::addressbook::contact new_contact;
              if( !con.icon.isNull() )
              {
                  QImage image;
                  QByteArray ba;
                  QBuffer buffer(&ba);
                  buffer.open(QIODevice::WriteOnly);
                  image.save(&buffer, "PNG"); // writes image into ba in PNG format
              }

              return new_contact;
          }
    };
}



AddressBookModel::AddressBookModel( QObject* parent, bts::addressbook::addressbook_ptr abook )
:QAbstractTableModel(parent),my( new Detail::AddressBookModelImpl() )
{
   my->_abook = abook;
   my->_default_icon.addFile(QStringLiteral(":/images/user.png"), QSize(), QIcon::Normal, QIcon::Off);

   const std::unordered_map<uint32_t,bts::addressbook::contact>& loaded_contacts = abook->get_contacts();
   my->_contacts.reserve( loaded_contacts.size() );
   for( auto itr = loaded_contacts.begin(); itr != loaded_contacts.end(); ++itr )
   {
      my->_contacts.push_back( my->convert_contact( itr->second ) );
   }
}

AddressBookModel::~AddressBookModel()
{
}

int AddressBookModel::rowCount( const QModelIndex& parent )const
{
    return my->_contacts.size();
}

int AddressBookModel::columnCount( const QModelIndex& parent  )const
{
    return NumColumns;
}

bool AddressBookModel::removeRows( int row, int count, const QModelIndex& parent )
{
   return false;
}

QVariant AddressBookModel::headerData( int section, Qt::Orientation orientation, int role )const
{
    if( orientation == Qt::Horizontal )
    {
       switch( role )
       {
          case Qt::DecorationRole:
             switch( (Columns)section )
             {
                case UserIcon:
                    return my->_default_icon;
                default:
                   return QVariant();
             }
          case Qt::DisplayRole:
          {
              switch( (Columns)section )
              {
                 case FirstName:
                     return tr("First Name");
                 case LastName:
                     return tr("Last Name");
                 case Id:
                     return tr("Id");
                 case Age:
                     return tr("Age");
                 case Repute:
                     return tr("Repute");
                 case UserIcon:
                 case NumColumns:
                     break;
              }
          }
          case Qt::SizeHintRole:
              switch( (Columns)section )
              {
                  case UserIcon:
                      return QSize( 32, 16 );
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

QVariant AddressBookModel::data( const QModelIndex& index, int role )const
{
    if( !index.isValid() ) return QVariant();

    const Contact& current_contact = my->_contacts[index.row()];
    switch( role )
    {
       case Qt::SizeHintRole:
           switch( (Columns)index.column() )
           {
               case UserIcon:
                   return QSize( 48, 48 );
               default:
                   return QVariant();
           }
       case Qt::DecorationRole:
          switch( (Columns)index.column() )
          {
             case UserIcon:
                 if( current_contact.icon.isNull() ) 
                    return my->_default_icon;
                 return current_contact.icon;
             default:
                return QVariant();
          }
       case Qt::DisplayRole:
          switch( (Columns)index.column() )
          {
             case FirstName:
                 return current_contact.first_name;
             case LastName:
                 return current_contact.last_name;
             case Id:
                 return current_contact.bit_id;
             case Age:
                 return 0;
             case Repute:
                 return 0;

             case UserIcon:
             case NumColumns:
                return QVariant();
          }
    }
    return QVariant();
}

int AddressBookModel::storeContact( const Contact& contact_to_store )
{
   if( contact_to_store.wallet_account_index == -1 )
   {
       auto num_contacts = my->_contacts.size();
       beginInsertRows( QModelIndex(), num_contacts, num_contacts );
          my->_contacts.push_back(contact_to_store);
          my->_contacts.back().wallet_account_index =  my->_contacts.size()-1;
       endInsertRows();
       // TODO: store to disk...
       return my->_contacts.back().wallet_account_index;
   }
   else
   {
       FC_ASSERT( contact_to_store.wallet_account_index < int(my->_contacts.size()) );
       auto row = contact_to_store.wallet_account_index;
       my->_contacts[row] = contact_to_store;

       Q_EMIT dataChanged( index( row, 0 ), index( row, NumColumns - 1) );
       return contact_to_store.wallet_account_index;
   }
}

const Contact& AddressBookModel::getContactById( int contact_id )
{
   for( uint32_t i = 0; i < my->_contacts.size(); ++i )
   {
        if( my->_contacts[i].wallet_account_index == contact_id )
        {
            return my->_contacts[i];
        }
   }
}
const Contact& AddressBookModel::getContact( const QModelIndex& index  )
{
   FC_ASSERT(index.row() < (int)my->_contacts.size() );
   return my->_contacts[index.row()];
}

