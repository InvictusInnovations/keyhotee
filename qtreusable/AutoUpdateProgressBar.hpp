#ifndef __AUTOUPDATEPROGRESSBAR_HPP
#define __AUTOUPDATEPROGRESSBAR_HPP

#include <QProgressBar>

#include <functional>
#include <memory>

namespace fc
{
class exception;
} ///namespace fc

/** Autoupdating progress bar (timer based). Allows to safely update displayed value from another
    thread.
*/
class TAutoUpdateProgressBar : protected QProgressBar
  {
  Q_OBJECT

  public:
    /** Allows to create and show a progress bar control.
        Built object must be released by calling Release at the end to hide it and destroy
        the object created for it.
        \param rect   - describes initial size & position of the widget,
        \param title  - title to be displayed,
        \param max    - max. displayed value (when progress bar is completely filled in),
        \param parent - optional parent for created widget
    */
    static TAutoUpdateProgressBar* create(const QRect& rect, const QString& title, unsigned int max,
      QWidget* parent = nullptr);

    void doTask(std::function<void()> mainTask, std::function<void()> onFinish,
      std::function<void(QString)> onError);

    /** Allows to update value displayed for current progress bar.
    */
    void updateValue(int value);

  private:
    class TUpdateNotifier;
    TAutoUpdateProgressBar(unsigned int maxValue = 0, QWidget* parent = nullptr);
    virtual ~TAutoUpdateProgressBar();
    /** Allows to hide & destroy current object. Private since widget can be hidden automatically when
        mainTask finishes.
    */
    void release();

  private slots:
    void onFinish();
    void onFinishWithError(QString errorInfo);

  /// Class attributes:
  private:
    TUpdateNotifier*             _notifier;
    std::function<void()>        _onFinishAction;
    std::function<void(QString)> _onErrorAction;
    int                          _maxValue;
  };

#endif ///__AUTOUPDATEPROGRESSBAR_HPP


