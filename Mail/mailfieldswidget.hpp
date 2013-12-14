#ifndef MAILFIELDSWIDGET_HPP
#define MAILFIELDSWIDGET_HPP

#include <QWidget>

namespace Ui 
{
class MailFieldsWidget;
}

class MailEditorMainWindow;

class MailFieldsWidget : public QWidget
  {
    Q_OBJECT

  public:
    explicit MailFieldsWidget(MailEditorMainWindow& parent);
    virtual ~MailFieldsWidget();

    void showFromControls(bool show);
    void showCcControls(bool show);
    void showBccControls(bool show);

  private:
    /// Allows to show/hide given layout & all widgets associated with it.
    void showChildLayout(QLayout* layout, bool show, int preferredPosition);
    /// Helper for showChildLayout.
    void showLayoutWidgets(QLayout* layout, bool show);

  private slots:
    void on_sendButton_clicked();

  private:
    MailEditorMainWindow& MainEditor;
    Ui::MailFieldsWidget *ui;
  };

#endif // MAILFIELDSWIDGET_HPP
