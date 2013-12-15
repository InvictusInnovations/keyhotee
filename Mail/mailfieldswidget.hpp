#ifndef MAILFIELDSWIDGET_HPP
#define MAILFIELDSWIDGET_HPP

#include <QWidget>

namespace Ui 
{
class MailFieldsWidget;
}

class MailFieldsWidget : public QWidget
  {
    Q_OBJECT

  public:
    MailFieldsWidget(QWidget& parent, QAction& actionSend);
    virtual ~MailFieldsWidget();

    void showFromControls(bool show);
    void showCcControls(bool show);
    void showBccControls(bool show);

  private:
    /// Allows to show/hide given layout & all widgets associated with it.
    void showChildLayout(QLayout* layout, bool show, int preferredPosition);
    /// Helper for showChildLayout.
    void showLayoutWidgets(QLayout* layout, bool show);
    void validateSendButtonState();

  private slots:
    void on_sendButton_clicked();

    void on_toEdit_textChanged(const QString &arg1);

    void on_bccEdit_textChanged(const QString &arg1);

    void on_ccEdit_textChanged(const QString &arg1);

  private:
    Ui::MailFieldsWidget *ui;
    QAction&              ActionSend;
  };

#endif // MAILFIELDSWIDGET_HPP
