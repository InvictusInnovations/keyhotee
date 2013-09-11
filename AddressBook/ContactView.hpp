#pragma once
#include <QWidget>
#include <memory>

namespace Ui { class ContactView; }

class ContactView : public QWidget
{
  public:
     ContactView( QWidget* parent = nullptr );
     ~ContactView();

  private:
     std::unique_ptr<Ui::ContactView> ui;
};
