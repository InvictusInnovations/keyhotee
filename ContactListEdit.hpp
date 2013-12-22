#pragma once
#include <QTextEdit>

#include "ch/mailprocessor.hpp"

class QCompleter;

/**
 * @brief provides an implementation 'smart addresses' with auto-complete
 * based upon a QCompleter populated with all known dac-id's and contact names.
 */
class ContactListEdit : public QTextEdit
{
  Q_OBJECT
public:
  ContactListEdit(QWidget* parent = nullptr);
  virtual ~ContactListEdit();

  void setCompleter(QCompleter* completer);

  /** Allows to explicitly show completer control and start search with given completion prefix.
  */
  void showCompleter(const QString& completionPrefix);

  /// Allows to fill control with previously collected list of contacts.
  void SetCollectedContacts(const IMailProcessor::TRecipientPublicKeys& storage);
  /// Allows to get all collected contacts
  void GetCollectedContacts(IMailProcessor::TRecipientPublicKeys* storage) const;

  QSize sizeHint() const;
  QSize maximumSizeHint() const
    {
    return sizeHint();
    }

  bool focusNextPrevChild(bool);
protected:
  void keyPressEvent(QKeyEvent* key_event);
  void focusInEvent(QFocusEvent* focus_event);
  void resizeEvent(QResizeEvent* resize_event);

public Q_SLOTS:
  void insertCompletion( const QString& completion, bool isKeyhoteeFounder = false);
  void insertCompletion( const QModelIndex& completion );
  /// Slot to explicitly request to show a completer.
  void onCompleterRequest();

private Q_SLOTS:
  void fitHeightToDocument();

private:
  QString     textUnderCursor() const;
  QStringList getListOfImageNames() const;
  void        addContactEntry(const QString& contactText, bool isKeyhoteeFounder);

private:
  int         _fitted_height;
  QCompleter* _completer;
};
