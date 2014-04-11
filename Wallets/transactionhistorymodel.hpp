#ifndef TRANSACTIONHISTORYMODEL_HPP
#define TRANSACTIONHISTORYMODEL_HPP

#include <QAbstractTableModel>
#include <memory>

namespace Detail { class TransactionHistoryModelImpl; }

namespace Wallet {

/**
 * @brief The TransactionHistoryModel class provides a model for the transaction
 * history of a given wallet.
 */
class TransactionHistoryModel : public QAbstractTableModel {
public:
    TransactionHistoryModel();
    virtual ~TransactionHistoryModel();

    enum Columns {
        Status,
        Date,
        Memo,
        Amount,
        Balance,
        NumColumns
    };

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;


private:
    //std::unique_ptr<Detail::TransactionHistoryModelImpl> my;
    Detail::TransactionHistoryModelImpl *my;
};

} // namespace Wallet

#endif // TRANSACTIONHISTORYMODEL_HPP
