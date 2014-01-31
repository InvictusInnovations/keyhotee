#pragma once
#include <QDialog>

namespace Ui {
class RequestAuthorization;
}

class RequestAuthorization : public QDialog
{
  Q_OBJECT

public:
  explicit RequestAuthorization(QWidget *parent = 0);
  ~RequestAuthorization();

  void setKeyhoteeID(const QString& name);
  void setPublicKey(const QString& name);

  void enableAddContact(bool active);

  void onSend();

private:
  Ui::RequestAuthorization *ui;
};

