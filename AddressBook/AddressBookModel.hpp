#pragma once

#include <QAbstractTableModel>
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
    Ownership,
    FirstName,
    LastName,
    Id,
    Age,
    Repute,
    Authorization,
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
