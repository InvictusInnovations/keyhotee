#pragma once
#include <QtGui>
#include <bts/profile.hpp>

class MessageHeader;

namespace Detail { class InboxModelImpl; }

class InboxModel : public QAbstractTableModel
{
  public:
    InboxModel(QObject* parent, const bts::profile_ptr& user_profile, bts::bitchat::message_db_ptr mail_db);
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

    void addMailHeader(const bts::bitchat::message_header& header);
    void getFullMessage( const QModelIndex& index, MessageHeader& header )const;

    virtual int rowCount( const QModelIndex& parent = QModelIndex() )const;
    virtual int columnCount( const QModelIndex& parent = QModelIndex() )const;

    virtual bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() );

    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole )const;
    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole )const;


  private:
     void fillMailHeader(const bts::bitchat::message_header& header,
                         MessageHeader& mail_header);

     void readMailBoxHeadersDb(bts::bitchat::message_db_ptr mail_db);

     std::unique_ptr<Detail::InboxModelImpl> my;
};
