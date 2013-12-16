#pragma once
#include <QTextEdit>

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
  ~ContactListEdit();

  void setCompleter(QCompleter* completer);
  QCompleter* getCompleter();

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
       void insertCompletion( const QString& completion, bool isKeyhoteeFounder = false );
       void insertCompletion( const QModelIndex& completion );
private Q_SLOTS:
  void fitHeightToDocument();

private:
  QString textUnderCursor() const;

private:
  int         _fitted_height;
  QCompleter* _completer;
};
