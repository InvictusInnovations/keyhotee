#ifndef HEADERWIDGET_H
#define HEADERWIDGET_H

#include <QWidget>
#include <QString>

namespace Ui {
class HeaderWidget;
}
/** Class used to display a title bar in the ContactView
    and MailView
*/
class HeaderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HeaderWidget(QWidget *parent);
    ~HeaderWidget();
    void initial(QString title);

private:
    Ui::HeaderWidget *ui;
};

#endif // HEADERWIDGET_H
