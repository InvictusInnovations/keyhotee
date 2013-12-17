#ifndef MAILFIELDSWIDGET_HPP
#define MAILFIELDSWIDGET_HPP

#include <bts/profile.hpp>

#include <QWidget>

#include <map>

namespace Ui 
{
class MailFieldsWidget;
}

class AddressBookModel;

class MailFieldsWidget : public QWidget
  {
  Q_OBJECT

  public:
    MailFieldsWidget(QWidget& parent, QAction& actionSend, AddressBookModel& abModel);
    virtual ~MailFieldsWidget();

    void showFromControls(bool show);
    void showCcControls(bool show);
    void showBccControls(bool show);

    /// Returns currently set subject text.
    QString getSubject() const;
    const bts::identity& getSelectedSenderIdentity() const
      {
      return SenderIdentity;
      }

  Q_SIGNAL void subjectChanged(const QString& subject);

  private:
    /// Allows to show/hide given layout & all widgets associated with it.
    void showChildLayout(QLayout* layout, bool show, int preferredPosition);
    /// Helper for showChildLayout.
    void showLayoutWidgets(QLayout* layout, bool show);
    void validateSendButtonState();
    void fillSenderIdentities();

  private slots:
    void on_sendButton_clicked();
    void onRecipientListChanged();
    void onSubjectChanged(const QString& subject);
    void onFromBtnTriggered(QAction* action);

  private:
    typedef std::map<QAction*, bts::identity> TAction2IdentityIndex;
    Ui::MailFieldsWidget *ui;
    QAction&              ActionSend;
    /// Helper map to associate action for created 'from-sub-menu' item to given identity.
    TAction2IdentityIndex Action2Identity;
    bts::identity         SenderIdentity;
  };

#endif // MAILFIELDSWIDGET_HPP
