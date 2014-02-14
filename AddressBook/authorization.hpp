#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include <QWidget>
#include <bts/profile.hpp>

namespace Ui {
class Authorization;
}
class QToolBar;
class AuthorizationItem;

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
  void setOwnerItem(AuthorizationItem* item);

private:
  Ui::Authorization *ui;

  QToolBar*             _toolbar;
  QAction*              _accept;
  QAction*              _deny;
  QAction*              _block;
  AuthorizationItem*    _owner_item;
};

#endif // AUTHORIZATION_H
