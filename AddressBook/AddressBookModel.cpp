#include "AddressBookModel.hpp"

#include "Contact.hpp"

#include <QIcon>

#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/raw.hpp>

bool FilterBlockedModel::_is_filter_on = false;

namespace Detail
{

class AddressBookModelImpl
{
  public:
    QIcon                             _default_icon;
    QIcon                             _ownership_yes;
    QIcon                             _authorized;
    QIcon                             _blocked;
    QIcon                             _autho_sent;
    QIcon                             _blocked_me;
    QIcon                             _unauthorized;
    std::vector<Contact>              _contacts;
    bts::addressbook::addressbook_ptr _address_book;
//    std::vector<int>                  _completion_row_to_wallet_index;
};

} /// namespace Detail

AddressBookModel::AddressBookModel(QObject* parent, bts::addressbook::addressbook_ptr address_book)
  : QAbstractTableModel(parent), my(new Detail::AddressBookModelImpl() )
{
  my->_address_book = address_book;
  my->_default_icon.addFile(QStringLiteral(":/images/user.png"), QSize(), QIcon::Normal, QIcon::Off);

  my->_ownership_yes.addFile(QStringLiteral(":/images/ownership.png"), QSize(), QIcon::Normal, QIcon::Off);
  
  my->_blocked.addFile(QStringLiteral(":/images/request_block.png"), QSize(), QIcon::Normal, QIcon::Off);
  my->_authorized.addFile(QStringLiteral(":/images/request_accept.png"), QSize(), QIcon::Normal, QIcon::Off);
  my->_autho_sent.addFile(QStringLiteral(":/images/request_sent.png"), QSize(), QIcon::Normal, QIcon::Off);
  my->_blocked_me.addFile(QStringLiteral(":/images/request_blocked_me.png"), QSize(), QIcon::Normal, QIcon::Off);
  my->_unauthorized.addFile(QStringLiteral(":/images/request_unauthorized.png"), QSize(), QIcon::Normal, QIcon::Off);

  reloadContacts();
}

AddressBookModel::~AddressBookModel()
{}

int AddressBookModel::rowCount(const QModelIndex& parent) const
{
  return static_cast<int>(my->_contacts.size());
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
  //remove from in-memory contact list
  auto rowI = my->_contacts.begin() + row;
  my->_contacts.erase(rowI, rowI + count);
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
          case LastName:
            return tr("Last Name");
          case Id:
            return tr("Id");
          case Age:
            return tr("Age");
          case Repute:
            return tr("Repute");
          default:
            return QVariant();
        }
      }
    case Qt::TextAlignmentRole:
      switch ((Columns)section)
      {
        case FirstName:
        case LastName:
        case Id:
          return Qt::AlignLeft + Qt::AlignVCenter;
        case Age:
        case Repute:
          return Qt::AlignRight + Qt::AlignVCenter;
        default:
          return QVariant();
      } //switch columns in TextAlignmentRole
    case Qt::SizeHintRole:
      switch ( (Columns)section)
      {
      case UserIcon:
        return QSize(32, 16);
      default:
        return QVariant();
      } // switch column in SizeHintRole
    case Qt::ToolTipRole:
      switch((Columns)section)
      {
      case ContactStatus:
        return tr("Contact status");
      default:
        return QVariant();
      } //switch column in ToolTipRole
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
        case ContactStatus:
          return QSize(48, 48);
        default:
          return QVariant();
      } //switch column in SizeHintRole
    case Qt::DecorationRole:
      switch ( (Columns)index.column() )
      {
        case UserIcon:
          return current_contact.getIcon();
        case ContactStatus:
          if(current_contact.isOwn())
            return my->_ownership_yes;
          else if(current_contact.auth_status == bts::addressbook::i_block)
            return my->_blocked;
          else if(current_contact.auth_status == bts::addressbook::accepted ||
                  current_contact.auth_status == bts::addressbook::accepted_chat ||
                  current_contact.auth_status == bts::addressbook::accepted_mail)
            return my->_authorized;
          else if(current_contact.auth_status == bts::addressbook::sent_request)
            return my->_autho_sent;
          else if(current_contact.auth_status == bts::addressbook::blocked_me)
            return my->_blocked_me;
          else if(current_contact.auth_status == bts::addressbook::unauthorized ||
                  current_contact.auth_status == bts::addressbook::denied)
            return my->_unauthorized;
          else
            return QVariant();
        default:
          return QVariant();
      } //switch column in DecorationRole
    case Qt::DisplayRole:
      switch ( (Columns)index.column() )
      {
        case FirstName:
          return current_contact.first_name.c_str();
        case LastName:
          return current_contact.last_name.c_str();
        case Id:
          if(current_contact.dac_id_string.empty() &&
            current_contact.first_name.empty() && current_contact.last_name.empty())
            return current_contact.get_trim_pk().c_str();
          else
            return current_contact.dac_id_string.c_str();
        case Age:
          return current_contact.getAge();
        case Repute:
          return current_contact.getRepute();
        case ContactStatus:
          return QVariant();
        default:
          return QVariant();
      } //switch column in DisplayRole
    case Qt::UserRole:
      switch ( (Columns)index.column() )
      {
        case ContactStatus:
          return current_contact.isBlocked();
        case FirstName:
          return current_contact.first_name.c_str();
        case LastName:
          return current_contact.last_name.c_str();
        case Id:
          if(current_contact.dac_id_string.empty() &&
            current_contact.first_name.empty() && current_contact.last_name.empty())
            return current_contact.get_trim_pk().c_str();
          else
            return current_contact.dac_id_string.c_str();
        case Age:
          return current_contact.getAge();
        case Repute:
          return current_contact.getRepute();
        default:
          return QVariant();
      } //switch column in UserRole
    case Qt::TextAlignmentRole:
      switch ((Columns)index.column())
      {
        case FirstName:
        case LastName:
        case Id:
          return Qt::AlignLeft + Qt::AlignVCenter;
        case Age:
        case Repute:
          return Qt::AlignRight + Qt::AlignVCenter;
        default:
          return QVariant();
      } //switch columns in TextAlignmentRole
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
        case ContactStatus:
          if(current_contact.isOwn())
            return tr("Ownership");
          else if(current_contact.auth_status == bts::addressbook::i_block)
            return tr("Blocked");
          else if(current_contact.auth_status == bts::addressbook::accepted)
            return tr("Authorized");
          else if(current_contact.auth_status == bts::addressbook::accepted_chat)
            return tr("Authorized chat");
          else if(current_contact.auth_status == bts::addressbook::accepted_mail)
            return tr("Authorized mail");
          else if(current_contact.auth_status == bts::addressbook::sent_request)
            return tr("Request sent");
          else if(current_contact.auth_status == bts::addressbook::blocked_me)
            return tr("Blocked me");
          else if(current_contact.auth_status == bts::addressbook::unauthorized)
            return tr("Unauthorized");
          else if(current_contact.auth_status == bts::addressbook::denied)
            return tr("Denied");
          else
            return QVariant();
        default:
          return QVariant();
      } //switch column in ToolTipRole
  } //switch

  return QVariant();
}

int AddressBookModel::storeContact(const Contact& contact_to_store)
{
  QModelIndex completionIndex;
  if (contact_to_store.wallet_index == WALLET_INVALID_INDEX)
  {
    //Start at 0 if no contacts, otherwise use next higher wallet index than currently exists.
    //Contacts are sorted by wallet index, so highest index is at back of list
    auto next_wallet_index = my->_contacts.empty() ? 0 : my->_contacts.back().wallet_index + 1;
    auto num_contacts = my->_contacts.size();
    beginInsertRows(QModelIndex(), num_contacts, num_contacts);
    my->_contacts.push_back(contact_to_store);
    my->_contacts.back().wallet_index = next_wallet_index;
    endInsertRows();

    //add fullname to completion list
    my->_address_book->store_contact(my->_contacts.back() );
    return next_wallet_index;
  }

  //FC_ASSERT(contact_to_store.wallet_index < int(my->_contacts.size()) );
  /*
  auto row = -1;
  for (uint32_t i = 0; i < my->_contacts.size(); ++i)
    if (my->_contacts[i].wallet_index == contact_to_store.wallet_index)
    {
       row = i; 
       break;
    }
  if(row == -1)
    FC_ASSERT(!"invalid contact id");
    */
  //find contact using binary search as they are stored sorted by wallet_index
  auto row = getContactRow(contact_to_store);
  my->_contacts[row] = contact_to_store;
  my->_address_book->store_contact(my->_contacts[row]);

  Q_EMIT dataChanged(index(row, 0), index(row, NumColumns - 1) );
  return contact_to_store.wallet_index;
}

int AddressBookModel::getContactRow(const Contact& contact) const
{
  std::vector<Contact>::const_iterator contact_iterator = std::lower_bound(my->_contacts.begin(),
    my->_contacts.end(), contact);

  FC_ASSERT(contact_iterator != my->_contacts.end(), "invalid contact id");
  FC_ASSERT(contact_iterator->wallet_index == contact.wallet_index,
    "invalid contact id (wallet_index mismatch)");

  return contact_iterator - my->_contacts.begin();
}

const Contact& AddressBookModel::getContactById(int contact_id) const
{
  Contact temp_contact;
  temp_contact.wallet_index = contact_id;
  int row  = getContactRow(temp_contact);
  return my->_contacts[row];
  /*
  for (uint32_t i = 0; i < my->_contacts.size(); ++i)
    if (my->_contacts[i].wallet_index == contact_id)
      return my->_contacts[i];
  FC_ASSERT(!"invalid contact id");
  */
  //FC_ASSERT( !"invalid contact id ${id}", ("id",contact_id) );
}

const Contact& AddressBookModel::getContact(const QModelIndex& index) const
{
  FC_ASSERT(index.row() < (int)my->_contacts.size() );
  return my->_contacts[index.row()];
}

void AddressBookModel::reloadContacts()
{
  const std::unordered_map<uint32_t, bts::addressbook::wallet_contact>& loaded_contacts =
    my->_address_book->get_contacts();
  my->_contacts.clear();
  my->_contacts.reserve(loaded_contacts.size() );

  for (auto itr = loaded_contacts.begin(); itr != loaded_contacts.end(); ++itr)
  {
    auto contact = itr->second;
    ilog("loading contacts...");
    my->_contacts.push_back(Contact(contact) );
  }
  std::sort( my->_contacts.begin(), my->_contacts.end());
}

QModelIndex AddressBookModel::findModelIndex(const int wallet_index) const
{
  Contact temp_contact;
  temp_contact.wallet_index = wallet_index;
  int row  = getContactRow(temp_contact);
  
  if(row < 0)
    return QModelIndex();
  else
    return index(row, 0);
}

