#include "AddressBookModel.hpp"

#include <fc/log/logger.hpp>

namespace Detail 
{
    class AddressBookModelImpl
    {
       public:
          std::vector<bts::addressbook::contact>  _contacts;
          bts::addressbook::addressbook_ptr       _abook;
    };
}



AddressBookModel::AddressBookModel( bts::addressbook::addressbook_ptr abook )
:my( new Detail::AddressBookModelImpl() )
{
   my->_abook = abook;
   auto known = abook->get_known_bitnames();
   my->_contacts.reserve(known.size());
   for( auto itr = known.begin(); itr != known.end(); ++itr )
   {
       auto opt_contact = my->_abook->get_contact_by_bitname( *itr );
       if( !opt_contact )
       {
          wlog( "broken addressbook, unable to find ${name} ", ("name",*itr) );
       }
       else
       {
          my->_contacts.push_back( *opt_contact );
       }
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
                 case NumColumns:
                     break;
              }
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
    if( !index.isValid() || role != Qt::DisplayRole ) return QVariant();

    const bts::addressbook::contact& current_contact = my->_contacts[index.row()];
    switch( (Columns)index.column() )
    {
       case FirstName:
           return current_contact.first_name.c_str();
       case LastName:
           return current_contact.last_name.c_str();
       case Id:
           return current_contact.bitname_id.c_str();
       case Age:
           return 0;
       case Repute:
           return 0;

       case NumColumns:
          return QVariant();
    }
}

void AddressBookModel::storeContact( const bts::addressbook::contact& new_contact )
{
   auto num_contacts = my->_contacts.size();
   if( num_contacts > 0 )
   {
      beginInsertRows( QModelIndex(), num_contacts, num_contacts );
      my->_contacts.push_back(new_contact);
      endInsertRows();
      my->_abook->store_contact( new_contact );
   }
}

