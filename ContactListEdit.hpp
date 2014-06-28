#pragma once
#include <QTextEdit>

#include "ch/mailprocessor.hpp"

#include <bts/addressbook/contact.hpp>

class QCompleter;
class QModelIndex;
class AddressBookModel;
class TContactCompletionModel;

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

  /** Allows to setup addressbook model to feed completer.
      \warning This method must be called before using other methods of this class. Best would be to
      extend class constructor by given argument, but it is impossible since this class is usually
      instantiated by widgets generated using QtDesigner.
  */
  void setAddressBookModel(AddressBookModel& abModel);

  /// Allows to fill control with previously collected list of contacts.
  void SetCollectedContacts(const IMailProcessor::TRecipientPublicKeys& storage);
  /// Allows to get all collected contacts
  void GetCollectedContacts(IMailProcessor::TRecipientPublicKeys* storage) const;

  QSize sizeHint() const;
  QSize maximumSizeHint() const
    {
    return sizeHint();
    }

public slots:
  /// Slot to explicitly request to show a completer.
  void onCompleterRequest();

private slots:
  void insertCompletion(const QModelIndex& completion);
  void onTextChanged();

  void fitHeightToDocument();
  void onActiveAddContact();
  void onActiveFindContact();

protected:
  virtual void keyPressEvent(QKeyEvent* key_event) override;
  virtual void focusInEvent(QFocusEvent* focus_event) override;
  virtual void resizeEvent(QResizeEvent* resize_event) override;
  /// Reimplemented to correctly support clipboard operations.
  virtual QMimeData *createMimeDataFromSelection() const override;
  virtual void contextMenuEvent ( QContextMenuEvent * event ) override;
  virtual bool focusNextPrevChild(bool) override;

private:
  /** Allows to explicitly show completer control and start search with given completion prefix.
      \warning Call initCompleter method before calling this method;
  */
  void showCompleter();
  /** Initializing completer.
      Set completion prefix and set popup current index
      Shoud be call before showCompleter();
  */
  void initCompleter(const QString& completionPrefix);
  QString textUnderCursor(QTextCursor* filledCursor = nullptr) const;
  //// Allows to delete entered text (pointed by text cursor position hold in _activeCursor).
  void    deleteEnteredText();
  void    addContactEntry(const QString& contactText, const bts::addressbook::contact& c,
    bool rawPublicKey, const QString* entryTooltip = nullptr);
  /// Stores public key as user def. property on the image to be created in document.
  void    encodePublicKey(const IMailProcessor::TRecipientPublicKey& key,
    QTextImageFormat* storage) const;
  /// Reads public key stored as user def. property from the image previously created in document.
  void    decodePublicKey(const QTextImageFormat& storage,
    IMailProcessor::TRecipientPublicKey* key) const;

  /** Allows to check if clicked position is contained by any text fragment pointing to contact.
      If so returns true. 
      @param position - click position to be checked
      @param isExistingContact - output, cannot be nullptr. Set to true if position points to some
                        contact and it is already defined in the address book.
      @param foundContact - output, cannot be nullptr. Filled with pointed contact data.
  */
  bool    validateClickPosition(const QPoint& position, bool* isExistingContact,
    bts::addressbook::wallet_contact* foundContact) const;

private:
  class TAutoSkipCompletion;

  const AddressBookModel*           _addressBookModel;
  TContactCompletionModel*          _completerModel;
  int                               _fitted_height;
  QCompleter*                       _completer;
  /// Holds data about contact which has been right clicked
  bts::addressbook::wallet_contact  _clicked_contact;
  /// Helper flag determining where textChanged signals should cause completer activation.
  bool                              _skipCompletion;
  /// Filled with selection range containing last entered text.
  QTextCursor                       _activeCursor;
};
