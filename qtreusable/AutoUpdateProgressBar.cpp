#include "qtreusable/AutoUpdateProgressBar.hpp"

#include <fc/log/logger.hpp>
#include <fc/thread/thread.hpp>

/// Helper notifier function, needed to transmit progress update signals between threads.
class TUpdateNotifier : public QObject
  {
  Q_OBJECT
  public:
    virtual ~TUpdateNotifier() {}

    /// Should be called to trigger progress bar update
    void updateProgress(int value)
      {
      emit onUpdateProgress(value);
      }

    void notifyFinished()
      {
      emit finished();
      }

  /// The client code should connect to this signal to make progress update safely.
  Q_SIGNALS:
    void onUpdateProgress(int value);
    void finished();
  };

TAutoUpdateProgressBar* 
TAutoUpdateProgressBar::create(const QRect& rect, const QString& title, unsigned int max,
  QWidget* parent /*= nullptr*/)
  {
  TAutoUpdateProgressBar* bar = new TAutoUpdateProgressBar(max, parent);
  bar->setWindowTitle(title);
  bar->setWindowFlags(Qt::WindowFlags(Qt::CustomizeWindowHint|Qt::WindowTitleHint));

  bar->_notifier = new TUpdateNotifier();

  connect(bar->_notifier, SIGNAL(onUpdateProgress(int)), bar, SLOT(setValue(int)), Qt::BlockingQueuedConnection);

  connect(bar->_notifier, SIGNAL(finished()), bar, SLOT(onFinish()));

#ifndef WIN32
  bar->move(rect.topLeft()); //doesn't take into account the width/height of the taskbar
#endif /// !WIN32
  bar->resize(rect.size());
  bar->show();

  ilog("Creating TAutoUpdateProgressBar");

  return bar;
  }

void TAutoUpdateProgressBar::doTask(std::function<void()> mainTask, std::function<void()> onFinish)
  {
  _onFinishAction = onFinish;
  
  ilog("Entering");

  fc::async([=]() {
    ilog("before mainTask()");
    mainTask();
    ilog("after mainTask()");
    _notifier->notifyFinished();
    });
  }

void TAutoUpdateProgressBar::release()
  {
  hide();
  deleteLater();
  }

void TAutoUpdateProgressBar::updateValue(int value)
  {
  ilog("Updating progress value...");
  _notifier->updateProgress(value);
  }

TAutoUpdateProgressBar::TAutoUpdateProgressBar(unsigned int maxValue, QWidget* parent) :
  QProgressBar(parent),
  _maxValue(maxValue)
  {
  setMaximum(maxValue);
  }

TAutoUpdateProgressBar::~TAutoUpdateProgressBar()
  {
  _notifier->deleteLater();
  }

void TAutoUpdateProgressBar::onFinish()
  {
  ilog("Entering...");
  setValue(_maxValue);
  release();
  ilog("executing _onFinishAction");
  _onFinishAction();
  }

#include "AutoUpdateProgressBar.moc"

