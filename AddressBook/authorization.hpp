#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include <QWidget>
#include <bts/profile.hpp>

namespace Ui {
class Authorization;
}

class Authorization : public QWidget
{
    Q_OBJECT

public:
    explicit Authorization(QWidget *parent = 0);
    ~Authorization();

    void onAccept();
    void onDeny();
    void onBlock();

    void setMsg(const bts::bitchat::decrypted_message& msg);

private:
    Ui::Authorization *ui;
};

#endif // AUTHORIZATION_H
