#pragma once
#include <QtGui>
#include <bts/profile.hpp>

namespace Detail { class InboxModelImpl; }

class MessageHeader
{
    public:
       MessageHeader():read_mark(false),attachment(false),money_amount(0){}

       QString     from;
       QIcon       from_icon;
       QString     to;
       QString     subject;
       QDateTime   date_received;
       QDateTime   date_sent;
       bool        read_mark;
       bool        attachment;
       QIcon       money_type;
       double      money_amount;

       fc::uint256  digest;
};

class InboxModel : public QAbstractTableModel
{
  public:
    InboxModel( QObject* parent, const bts::profile_ptr& user_profile );
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
        Status,
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
