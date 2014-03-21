#ifndef __INFOBAR_HPP
#define __INFOBAR_HPP

#include <QObject>

class QBoxLayout;
class QFrame;

/** Class implementing an info bar to be displayed into specifed cell of pointed
    box layout.
    Usually it could be displayed on top of current view, to show some additional infos.
    It allows to display some text and optional accept button. Always displays 'close'
    button just closing the bar without any additional action.
*/
class TInfoBar : public QObject
  {
  Q_OBJECT
  public:
    /** Allows to build an info bar object.
        \param textToDisplay - should be nonempty.
        \param acceptBtnText - optional, can be null. If not null also should be not empty
                               and leads to display a button sending 'accepted' signal.
        \param parent        - box layout where given info bar should be placed (displayed). Cannot
                               be null.
        \param index         - index to the part of specified layout where given bar should be put to.
    */
    static void showInfoBar(const QString& textToDisplay, const QString* acceptBtnText, QBoxLayout* parent,
      int index);

  signals:
    /// Sent when user clicks on accept button (if displayed).
    void infoBarAccepted();

  private:
    TInfoBar(QObject* objParent);
    virtual ~TInfoBar();

    void buildWidgets(const QString& textToDisplay, const QString* acceptBtnText, QBoxLayout* parent,
      int index);

  private slots:
    void cancelBtnClicked();

  /// Class attributes:
  private:
    QBoxLayout* _parent;
    QFrame*     _mainContainer;
  };

#endif /// __INFOBAR_HPP
