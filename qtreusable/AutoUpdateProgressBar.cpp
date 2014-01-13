#include "qtreusable/AutoUpdateProgressBar.hpp"

#include <fc/log/logger.hpp>
#include <fc/thread/thread.hpp>

typedef std::shared_ptr<fc::exception> TSharedException;

/// Helper notifier function, needed to transmit progress update signals between threads.
class TAutoUpdateProgressBar::TUpdateNotifier : public QObject
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

    /** Allows to notify receiver about caught exception.
        @param e - can be empty (without any resource attached) in case when unknown error has been
                   caught.
    */
    void notifyError(const fc::exception* e)
      {
      QString errorInfo;
      if(e != nullptr)
        errorInfo = QString::fromStdString(e->to_detail_string());

      emit finishedWithError(errorInfo);
      }

  /// The client code should connect to this signal to make progress update safely.
  Q_SIGNALS:
    void onUpdateProgress(int value);
    void finished();
    /// Emitted when some exception has been caught. @see notifyError for details.
    void finishedWithError(QString errorInfo);
  };

TAutoUpdateProgressBar* 
TAutoUpdateProgressBar::create(const QRect& rect, const QString& title, unsigned int max,
  QWidget* parent /*= nullptr*/)
  {
  TAutoUpdateProgressBar* bar = new TAutoUpdateProgressBar(max, parent);
  bar->setWindowTitle(title);
  bar->setWindowFlags(Qt::WindowFlags(Qt::CustomizeWindowHint|Qt::WindowTitleHint));

  bar->_notifier = new TUpdateNotifier();

  /** \warning Use here always Auto-Connection feature since even this bar is designed to execute
      main task in separate thread, using fc::async doesn't guarantee that new separate thread
      can be created. Then Qt::BlockingQueuedConnection can lead to lock (same thread).
      Reproduced on linux and also reported by some customer(s).
  */
  connect(bar->_notifier, SIGNAL(onUpdateProgress(int)), bar, SLOT(setValue(int)));
  connect(bar->_notifier, SIGNAL(finished()), bar, SLOT(onFinish()));
  connect(bar->_notifier, SIGNAL(finishedWithError(QString)), bar,
    SLOT(onFinishWithError(QString)), Qt::QueuedConnection);

#ifndef WIN32
  bar->move(rect.topLeft()); //doesn't take into account the width/height of the taskbar
#endif /// !WIN32
  bar->resize(rect.size());
  bar->show();

  ilog("Creating TAutoUpdateProgressBar");

  return bar;
  }

void TAutoUpdateProgressBar::doTask(std::function<void()> mainTask, std::function<void()> onFinish,
  std::function<void(QString)> onError)
  {
  _onFinishAction = onFinish;
  _onErrorAction = onError;

  ilog("Entering...");

  fc::async([=]() {
    try
      {
      ilog("before mainTask()");
      mainTask();
      ilog("after mainTask()");
      _notifier->notifyFinished();
      ilog("after notifyFinished()");
      }
    catch(const fc::exception& e)
      {
      elog("${e}", ("e", e.to_detail_string()));
      _notifier->notifyError(&e);
      }
    catch(...)
      {
      elog("unrecognized exception while creating profile");
      _notifier->notifyError(nullptr);
      }
    });

  ilog("Leaving...");
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
  ilog("Leaving...");
  }

void TAutoUpdateProgressBar::onFinishWithError(QString e)
  {
  ilog("Entering...");
  release();

  _onErrorAction(e);
  
  ilog("Performing a rethrow...");

  //if(e.isEmpty())
  //  throw *e.get();
  //else
  //  throw;
  }

#include "AutoUpdateProgressBar.moc"

