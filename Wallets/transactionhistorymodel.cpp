#include "transactionhistorymodel.hpp"
#include "wallettransaction.hpp"

class Detail::TransactionHistoryModelImpl {
public:
    QList<Wallet::WalletTransaction*> transactionHistory;
};

Wallet::TransactionHistoryModel::TransactionHistoryModel():
    my(new Detail::TransactionHistoryModelImpl)
{
}

Wallet::TransactionHistoryModel::~TransactionHistoryModel()
{}

int Wallet::TransactionHistoryModel::rowCount(const QModelIndex &parent) const
{
    return my->transactionHistory.length();
}

int Wallet::TransactionHistoryModel::columnCount(const QModelIndex &parent) const
{
    return NumColumns;
}

QVariant Wallet::TransactionHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    switch (Columns(section)) {
    case Status:
        return tr("Status");
    case Date:
        return tr("Date");
    case Memo:
        return tr("Memo");
    case Amount:
        return tr("Amount");
    case Balance:
        return tr("Balance");
    default:
        return QVariant();
    }
}

QVariant Wallet::TransactionHistoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole
            || index.row() < 0 || index.row() >= rowCount()
            || index.column() < 0 || index.column() >= columnCount())
        return QVariant();

    switch (Columns(index.column())) {
    case Status:
        return my->transactionHistory[index.row()]->status();
    case Date:
        return my->transactionHistory[index.row()]->date();
    case Memo:
        return my->transactionHistory[index.row()]->memo();
    case Amount:
        return my->transactionHistory[index.row()]->amount();
    case Balance:
        return my->transactionHistory[index.row()]->balance();
    default:
        return QVariant();
    }
}
