#pragma once
#include <QTextEdit>

#include "ch/mailprocessor.hpp"

namespace bts
{
namespace addressbook
{
struct contact;
struct wallet_contact;
} ///namespace addressbook
} /// namespace bts

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
  /// Reimplemented to correctly support clipboard operations.
  virtual QMimeData *createMimeDataFromSelection() const;
  virtual void contextMenuEvent ( QContextMenuEvent * event );

public Q_SLOTS:
  /// Slot to explicitly request to show a completer.
  void onCompleterRequest();

private Q_SLOTS:
  void insertCompletion(const QString& completion, const bts::addressbook::contact& c);
  void insertCompletion(const QModelIndex& completion);

  void fitHeightToDocument();
  void onActiveAddContact();
  void onActiveFindContact();

private:
  QString textUnderCursor() const;
  void    addContactEntry(const QString& contactText, const bts::addressbook::contact& c);
  /// Stores public key as user def. property on the image to be created in document.
  void    encodePublicKey(const IMailProcessor::TRecipientPublicKey& key,
    QTextImageFormat* storage) const;
  /// Reads public key stored as user def. property from the image previously created in document.
  void    decodePublicKey(const QTextImageFormat& storage,
    IMailProcessor::TRecipientPublicKey* key) const;
  bool    isClickOnContact();
  bool    isStoredContact();

private:
  int                               _fitted_height;
  QCompleter*                       _completer;  
  QPoint                            _right_click;
  QTextImageFormat                  _image_format;
  bts::addressbook::wallet_contact* _clicked_contact;
  mutable QString                   _prefixToDelete;
  QStringList                       _completions;
};
