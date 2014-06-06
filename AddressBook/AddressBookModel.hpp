#pragma once

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <bts/addressbook/addressbook.hpp>

namespace Detail { class AddressBookModelImpl; }

class Contact;

/**
 *  Provides a Model interface to the addressbook.
 */
class AddressBookModel : public QAbstractTableModel
{
public:
  AddressBookModel(QObject* parent, bts::addressbook::addressbook_ptr address_book);
  virtual ~AddressBookModel();

  enum Columns
    {
    UserIcon,
    ContactStatus,
    FirstName,
    LastName,
    Id,
    Age,
    Repute,
    NumColumns
    };
  //void storeContact( const bts::addressbook::contact& new_contact );

  /**
   *  @return the id assigned to this contact.
   */
  int storeContact(const Contact& new_contact);
  const Contact& getContactById(int contact_id) const;
  const Contact& getContact(const QModelIndex& index) const;

  QModelIndex findModelIndex(const int wallet_index) const;

/// QAbstractTableModel reimplementation:
  virtual int rowCount(const QModelIndex& parent = QModelIndex() ) const override;
  virtual int columnCount(const QModelIndex& parent = QModelIndex() ) const override;
  virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
  virtual QVariant headerData(int section, Qt::Orientation o, int role = Qt::DisplayRole) const override;
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

private:
  void reloadContacts();
  int getContactRow(const Contact& contact) const;

/// Class members:
  std::unique_ptr<Detail::AddressBookModelImpl> my;
};


class FilterBlockedModel : public QSortFilterProxyModel
{
public:
  FilterBlockedModel(QObject *parent = 0) : QSortFilterProxyModel(parent)
  {
    setFilterRole(Qt::UserRole);
    setFilterBlocked(true);
  }

  void setEnableFilter(bool b)
  {
    _is_filter_on = b;
    invalidateFilter();
  }

  void setFilterBlocked(bool b = true)
  {
    _is_filter_blocked_disable = b;
    invalidateFilter();
  }

  /** Returns true if "Enable filter blocked contacts" 
      parameter of menu option settings is active, else returns false
  */
  bool isFilterAvailable() const
  {
    return _is_filter_on;
  }
  /** Returns true if 'Show blocked contacts' Contact menu is enabled
      otherwise returns false
  */
  bool isFilterBlockedEnable() const
  {
    return (_is_filter_blocked_disable == false);
  }

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
  {
    if(!_is_filter_on)
      return true;

    QModelIndex blocked_index = sourceModel()->index(sourceRow, AddressBookModel::ContactStatus, sourceParent);
    return sourceModel()->data(blocked_index, filterRole()).toBool() != _is_filter_blocked_disable;
  }

private:
  bool _is_filter_blocked_disable;
  static bool _is_filter_on;
};