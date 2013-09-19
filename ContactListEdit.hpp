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
      ContactListEdit( QWidget* parent = nullptr );
      ~ContactListEdit();

      void setCompleter( QCompleter* c );
      QCompleter* getCompleter();

      QSize sizeHint() const;
      QSize maximumSizeHint() const { return sizeHint(); }
   protected:
       void keyPressEvent( QKeyEvent* e );
       void focusInEvent( QFocusEvent* e );
       void resizeEvent( QResizeEvent* e );
   
   private Q_SLOTS:
       void insertCompletion( const QString& completion );
       void fitHeightToDocument();

   private:
       QString textUnderCursor()const;

   private:
      int         _fitted_height;
      QCompleter* _completer;
};
