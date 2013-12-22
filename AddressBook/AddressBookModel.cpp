#include "AddressBookModel.hpp"
#include <QIcon>
#include <QPixmap>
#include <QImage>
#include <qcompleter.h>

#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/raw.hpp>
#include "Contact.hpp"



namespace Detail
{
class AddressBookModelImpl
  {
  public:
    QIcon                             _default_icon;
    QIcon                             _ownership_yes;
    QIcon                             _ownership_no;
    std::vector<Contact>              _contacts;
    bts::addressbook::addressbook_ptr _address_book;
    ContactCompletionModel            _contact_completion_model;
    QCompleter*                       _contact_completer;
  };
}

ContactCompletionModel::ContactCompletionModel(QObject* parent) 
  : QStringListModel(parent)
{
}
ContactCompletionModel::~ContactCompletionModel()
{
}
QVariant ContactCompletionModel::data( const QModelIndex& index, int role )const
{
  if( !index.isValid() )
    return QVariant();
  if (role == Qt::DisplayRole || role == Qt::EditRole)
   return QStringListModel::data(index, role);
  else if( role == Qt::UserRole )
    return index.row();
  return QVariant();
}


AddressBookModel::AddressBookModel(QObject* parent, bts::addressbook::addressbook_ptr address_book)
  : QAbstractTableModel(parent), my(new Detail::AddressBookModelImpl() )
  {
  my->_address_book = address_book;
  my->_default_icon.addFile(QStringLiteral(":/images/user.png"), QSize(), QIcon::Normal, QIcon::Off);

  my->_ownership_yes.addFile(QStringLiteral(":/images/ownership.png"), QSize(), QIcon::Normal, QIcon::Off);
  my->_ownership_no.addFile(QStringLiteral(":/images/blank.png"), QSize(), QIcon::Normal, QIcon::Off);

  const std::unordered_map<uint32_t, bts::addressbook::wallet_contact>& loaded_contacts = address_book->get_contacts();
  my->_contacts.reserve(loaded_contacts.size() );
  QStringList                                                           completion_list;
  for (auto itr = loaded_contacts.begin(); itr != loaded_contacts.end(); ++itr)
    {
    auto contact = itr->second;
    ilog("loading contacts...");
    my->_contacts.push_back(Contact(contact) );

    //add dac_id to completion list
    completion_list.push_back(contact.dac_id_string.c_str() );
    //add fullname to completion list
    QString fullName = contact.getFullName().c_str();
    completion_list.push_back(fullName);
    }
  my->_contact_completion_model.setStringList(completion_list);
  //create completer from completion model
  my->_contact_completer = new QCompleter(this);
  my->_contact_completer->setModel(&(my->_contact_completion_model) );
  //_contact_completer->setModelSorting( QCompleter::CaseInsensitivelySortedModel );
  my->_contact_completer->setCaseSensitivity(Qt::CaseInsensitive);
  my->_contact_completer->setWrapAround(true);
  }

AddressBookModel::~AddressBookModel()
  {}

int AddressBookModel::rowCount(const QModelIndex& parent) const
  {
  return my->_contacts.size();
  }

int AddressBookModel::columnCount(const QModelIndex& parent) const
  {
  return NumColumns;
  }

bool AddressBookModel::removeRows(int row, int count, const QModelIndex&)
  {
  beginRemoveRows(QModelIndex(), row, row + count - 1);
  // remove contacts from addressbook database
  for (int i = row; i < row + count; ++i)
    my->_address_book->remove_contact(my->_contacts[i]);
  for (int i = row; i < row + count; ++i)
    {
    // remove from addressbook database
    my->_address_book->remove_contact(my->_contacts[i]);
    }
  //remove from in-memory contact list
  auto rowI = my->_contacts.begin() + row;
  my->_contacts.erase(rowI, rowI + count);
  //remove fullname and dac_id from Qcompleter
  my->_contact_completion_model.removeRows(row * 2, count * 2);
  endRemoveRows();
  return true;
  }

QVariant AddressBookModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
  if (orientation == Qt::Horizontal)
    {
    switch (role)
      {
    case Qt::DecorationRole:
      switch ( (Columns)section)
        {
      case UserIcon:
        return my->_default_icon;
      default:
        return QVariant();
        }
    case Qt::DisplayRole:
        {
        switch ( (Columns)section)
          {
        case FirstName:
          return tr("First Name");
        case Ownership:
          return tr(" ");  // Ownership
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
      switch ( (Columns)section)
        {
      case UserIcon:
        return QSize(32, 16);
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

QVariant AddressBookModel::data(const QModelIndex& index, int role) const
  {
  if (!index.isValid() )
    return QVariant();
  const Contact& current_contact = my->_contacts[index.row()];
  switch (role)
    {
    case Qt::SizeHintRole:
      switch ( (Columns)index.column() )
        {
        case UserIcon:
          return QSize(48, 48);
        case Ownership:
          return QSize(32, 32);
        default:
          return QVariant();
        }
    case Qt::DecorationRole:
      switch ( (Columns)index.column() )
        {
        case UserIcon:
          return current_contact.getIcon();
        case Ownership:
          return current_contact.isOwn() ? my->_ownership_yes : my->_ownership_no;
        default:
          return QVariant();
        }
    case Qt::DisplayRole:
      switch ( (Columns)index.column() )
        {
        case FirstName:
          return current_contact.first_name.c_str();
        case LastName:
          return current_contact.last_name.c_str();
        case Id:
          return current_contact.dac_id_string.c_str();
        case Age:
          return current_contact.getAge();
        case Repute:
          return current_contact.getRepute();
        case Ownership:
        case UserIcon:
        case NumColumns:
          return QVariant();
        }
    case Qt::UserRole:
      switch ( (Columns)index.column() )
        {
        case Ownership:
          return current_contact.isOwn() ? true : false;
        case FirstName:
          return current_contact.first_name.c_str();
        case LastName:
          return current_contact.last_name.c_str();
        case Id:
          return current_contact.dac_id_string.c_str();
        case Age:
          return current_contact.getAge();
        case Repute:
          return current_contact.getRepute();
        default:
          return QVariant();
        }
    case Qt::BackgroundRole:
      if (current_contact.isKeyhoteeFounder())
        {
        return QVariant(QColor(231, 190, 66));
        }
      else
        {
        return QVariant();
        }
    case Qt::ToolTipRole:
      switch ( (Columns)index.column() )
        {
        case Ownership:
          return tr("Ownership");
        default:
          return QVariant();
        }
    } //switch

  return QVariant();
  }

int AddressBookModel::storeContact(const Contact& contact_to_store)
  {
  QModelIndex completionIndex;
  if (contact_to_store.wallet_index == WALLET_INVALID_INDEX)
    {
    auto                 num_contacts = my->_contacts.size();
    beginInsertRows(QModelIndex(), num_contacts, num_contacts);
    my->_contacts.push_back(contact_to_store);
    my->_contacts.back().wallet_index = my->_contacts.size() - 1;
    endInsertRows();
    //update completion model with new contact dac_id and fullname
    int                  row_count = my->_contact_completion_model.rowCount();
    my->_contact_completion_model.insertRows(row_count, 2);
    completionIndex = my->_contact_completion_model.index(row_count);
    my->_contact_completion_model.setData(completionIndex, contact_to_store.dac_id_string.c_str());
    completionIndex = my->_contact_completion_model.index(row_count + 1);
    my->_contact_completion_model.setData(completionIndex, contact_to_store.getFullName().c_str());

    //add fullname to completion list
    my->_address_book->store_contact(my->_contacts.back() );
    return my->_contacts.back().wallet_index;
    }

  FC_ASSERT(contact_to_store.wallet_index < int(my->_contacts.size()) );
  auto row = contact_to_store.wallet_index;
  my->_contacts[row] = contact_to_store;
  my->_address_book->store_contact(my->_contacts[row]);

  //update completion model with modified contact dac_id and fullname
  int completionRow = row * 2;
  completionIndex = my->_contact_completion_model.index(completionRow);
  my->_contact_completion_model.setData(completionIndex, contact_to_store.dac_id_string.c_str());
  completionIndex = my->_contact_completion_model.index(completionRow + 1);
  my->_contact_completion_model.setData(completionIndex, contact_to_store.getFullName().c_str());

  Q_EMIT dataChanged(index(row, 0), index(row, NumColumns - 1) );
  return contact_to_store.wallet_index;
  }

const Contact& AddressBookModel::getContactById(int contact_id)
  {
  for (uint32_t i = 0; i < my->_contacts.size(); ++i)
    if (my->_contacts[i].wallet_index == contact_id)
      return my->_contacts[i];

  FC_ASSERT(!"invalid contact id");
  //FC_ASSERT( !"invalid contact id ${id}", ("id",contact_id) );
  }

const Contact& AddressBookModel::getContact(const QModelIndex& index)
  {
  FC_ASSERT(index.row() < (int)my->_contacts.size() );
  return my->_contacts[index.row()];
  }

//QStringListModel* AddressBookModel::GetContactCompletionModel()
//{
//  return &(my->_contact_completion_model);
//}

QCompleter* AddressBookModel::getContactCompleter()
  {
  return my->_contact_completer;
  }

