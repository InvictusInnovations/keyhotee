#pragma once
#include <QtGui>
#include <bts/addressbook/addressbook.hpp>
#include "Contact.hpp"

namespace Detail { class AddressBookModelImpl; }

class AddressBookModel : public QAbstractTableModel
{
  public:
    AddressBookModel( QObject* parent, bts::addressbook::addressbook_ptr abook );
    ~AddressBookModel();

    enum Columns
    {
        UserIcon,
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
    int  storeContact( const Contact& new_contact );

    virtual int rowCount( const QModelIndex& parent = QModelIndex() )const;
    virtual int columnCount( const QModelIndex& parent = QModelIndex() )const;

    virtual bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() );

    virtual QVariant headerData( int section, Qt::Orientation o, int role = Qt::DisplayRole )const;
    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole )const;

  private:
     std::unique_ptr<Detail::AddressBookModelImpl> my;
};
