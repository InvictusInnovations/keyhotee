#pragma once
#include <QtGui>
#include <bts/bitchat/bitchat_message_db.hpp>

namespace Detail { class InboxModelImpl; }

class MessageHeader
{
    public:
};

class InboxModel : public QAbstractTableModel
{
  public:
    InboxModel( QObject* parent, const bts::bitchat::message_db_ptr& msg_db );
    ~InboxModel();

    enum Columns
    {
        Read,
        Money,
        Attachment,
        Chat,
        From,
        Subject,
        DateReceived,
        To,
        DateSent,
        NumColumns
    };

    bts::bitchat::decrypted_message getDecryptedMessage( const QModelIndex& index )const;  

    virtual int rowCount( const QModelIndex& parent = QModelIndex() )const;
    virtual int columnCount( const QModelIndex& parent = QModelIndex() )const;

    virtual bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() );

    virtual QVariant headerData( int section, Qt::Orientation o, int role = Qt::DisplayRole )const;
    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole )const;

  private:
     std::unique_ptr<Detail::InboxModelImpl> my;
};
