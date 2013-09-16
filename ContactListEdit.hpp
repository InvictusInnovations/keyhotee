#pragma once
#include <QTextEdit>

class QCompleter;

/**
 * @brief provides an implementation 'smart addresses' with auto-complete 
 * based upon a QCompleter populated with all known dac-id's and contact names.
 */
class ContactListEdit : public QTextEdit
{
   public:
      ContactListEdit( QWidget* parent = nullptr );
      ~ContactListEdit();

      void setCompleter( QCompleter* c );
      QCompleter* getCompleter();

   protected:
       void keyPressEvent( QKeyEvent* e );
       void focusInEvent( QFocusEvent* e );
   
   private Q_SLOTS:
       void insertCompletion( const QString& completion );

   private:
       QString textUnderCursor()const;

   private:
      QCompleter* _completer;
};
