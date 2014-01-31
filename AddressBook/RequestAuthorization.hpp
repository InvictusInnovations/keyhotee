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

  void onKeyhoteeIDChanged(const QString& name);
  void onPublicKeyChanged(const QString& name);

  void onSend();

private:
  Ui::RequestAuthorization *ui;
};

