#pragma once

#include <QString>
#include <QDateTime>

namespace Wallet {
    /**
     * @brief The WalletTransaction interface stores data relevant to a single transaction
     * in a wallet's history, for display in the GUI
     *
     * This is an interface class, and cannot be instantiated on its own. A wallet-specific
     * implementation must be provided.
     */
    class WalletTransaction {
    public:
        typedef double AmountType;
        enum TransactionStatus {
            Pending,
            Confirmed,
            NumStatuses
        };

        virtual TransactionStatus status() = 0;
        virtual QDateTime date() = 0;
        virtual QString memo() = 0;
        virtual void setMemo(const std::string &memoString) = 0;
        virtual AmountType amount() = 0;
        virtual AmountType balance() = 0;
    };
} // namespace Wallet
