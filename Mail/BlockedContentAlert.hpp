#ifndef BLOCKEDCONTENTALERT_HPP
#define BLOCKEDCONTENTALERT_HPP

#include <QWidget>

class IBlockerDelegate;

namespace Ui {
class BlockedContentAlert;
}

class BlockedContentAlert : public QWidget
{
    Q_OBJECT

public:
    explicit BlockedContentAlert(QWidget *parent = 0);
    ~BlockedContentAlert();

    void initial(IBlockerDelegate* blocker);

private slots:
    /// Display blocked remote images
    void onShowRemoteContent();

private:
    Ui::BlockedContentAlert *ui;
    IBlockerDelegate* _blocker;
};

#endif // BLOCKEDCONTENTALERT_HPP
