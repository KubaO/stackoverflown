#include <QApplication>
#include <QLabel>
#include <QGridLayout>
#include <QTableView>
#include <QHeaderView>
#include <QGroupBox>
#include <QThread>
#include <QDateTime>
#include <QElapsedTimer>
#include <QBasicTimer>
#include <QMutex>
#include <QMap>
#include <QStandardItemModel>
#include <QtConcurrent>
#include <QPainter>
#include <QSlider>
#include <random>

std::default_random_engine reng;

int ilog2(qint64 val) {
  Q_ASSERT(val >= 0);
  int ret = -1;
  while (val != 0) { val >>= 1; ret++; }
  return ret;
}

/// The value binned to contain at most \a binaryDigits significant digits.
/// The less significant digits are reset to zero.
qint64 binned(qint64 value, int binaryDigits)
{
  Q_ASSERT(binaryDigits > 0);
  qint64 mask = -1;
  int clrBits = ilog2(value) - binaryDigits;
  if (clrBits > 0) mask <<= clrBits;
  return value & mask;
}

/// A safely destructible thread for perusal by QObjects.
class Thread : public QThread {
  using QThread::run;
public:
  explicit Thread(QObject * parent = 0) : QThread(parent) {}
  ~Thread() { quit(); wait(); }
};

/// An application that monitors event loops in all threads.
class LoopMonitoringApp : public QApplication {
  Q_OBJECT
  Q_PROPERTY(int timeout READ timeout WRITE setTimeout MEMBER m_timeout)
  Q_PROPERTY(int updatePeriod READ updatePeriod WRITE setUpdatePeriod MEMBER m_updatePeriod)
public:
  typedef QMap<qint64, uint> Histogram;
  typedef QApplication Base;
private:
  struct ThreadData {
    /// A saturating, binned histogram of event handling durations for given thread.
    Histogram histogram;
    /// Number of milliseconds between the epoch and when the event handler on this thread
    /// was entered, or zero if no event handler is running.
    qint64 ping;
    /// Number of milliseconds between the epoch and when the last histogram update for
    /// this thread was broadcast
    qint64 update;
    /// Whether the thread's event loop is considered stuck at the moment
    bool stuck;
    ThreadData() : ping(0), update(0), stuck(false) {}
  };
  typedef QMap<QThread*, ThreadData> Threads;
  QMutex m_mutex;
  Threads m_threads;
  int m_timeout;
  int m_updatePeriod;

  class StuckEventLoopNotifier : public QObject {
    LoopMonitoringApp * m_app;
    QBasicTimer m_timer;
    void timerEvent(QTimerEvent * ev) Q_DECL_OVERRIDE {
      if (ev->timerId() != m_timer.timerId()) return;
      int timeout = m_app->m_timeout;
      auto now = QDateTime::currentMSecsSinceEpoch();
      QList<QPair<QThread*, int>> toEmit;
      QMutexLocker lock(&m_app->m_mutex);
      for (auto it = m_app->m_threads.begin(); it != m_app->m_threads.end(); ++it) {
        if (it->ping == 0) continue;
        qint64 elapsed = now - it->ping;
        if (elapsed > timeout) {
          it->stuck = true;
          toEmit << qMakePair(it.key(), elapsed);
        } else {
          if (it->stuck) toEmit << qMakePair(it.key(), 0);
          it->stuck = false;
        }
      }
      lock.unlock();
      for (auto & sig : toEmit) emit m_app->loopStateChanged(sig.first, sig.second);
    }
  public:
    explicit StuckEventLoopNotifier(LoopMonitoringApp * app) : m_app(app) {
      m_timer.start(100, Qt::CoarseTimer, this);
    }
  };
  StuckEventLoopNotifier m_notifier;
  Thread m_notifierThread;
  Q_SLOT void threadFinishedSlot() {
    auto const thread = qobject_cast<QThread*>(QObject::sender());
    QMutexLocker lock(&m_mutex);
    typename Threads::iterator it = m_threads.find(thread);
    if (it == m_threads.end()) return;
    auto const histogram(it->histogram);
    bool stuck = it->stuck;
    m_threads.erase(it);
    lock.unlock();
    emit newHistogram(thread, histogram);
    if (stuck) emit loopStateChanged(thread, 0);
    emit threadFinished(thread);
  }
protected:
  bool notify(QObject * receiver, QEvent * event) Q_DECL_OVERRIDE {
    auto const curThread = QThread::currentThread();
    QElapsedTimer timer;
    auto now = QDateTime::currentMSecsSinceEpoch();
    QMutexLocker lock(&m_mutex);
    typename Threads::iterator it = m_threads.find(curThread);
    bool newThread = it == m_threads.end();
    if (newThread) it = m_threads.insert(curThread, ThreadData());
    it->ping = now;
    lock.unlock();
    if (newThread) {
      QObject::connect(curThread, SIGNAL(finished()), this, SLOT(threadFinishedSlot()));
      QMetaObject::invokeMethod(this, "newThread", Qt::QueuedConnection,
                                Q_ARG(QThread*, curThread),
                                Q_ARG(QString, curThread->objectName()));
    }
    timer.start();
    auto result = Base::notify(receiver, event); // This is where the event loop can get "stuck".
    auto duration = binned(timer.elapsed(), 3);
    now += duration;
    lock.relock();
    auto & thread = m_threads[curThread];
    if (thread.histogram[duration] < std::numeric_limits<Histogram::mapped_type>::max())
      ++thread.histogram[duration];
    thread.ping = 0;
    qint64 sinceLastUpdate = now - thread.update;
    if (sinceLastUpdate >= m_updatePeriod) {
      auto const histogram = thread.histogram;
      thread.update = now;
      lock.unlock();
      emit newHistogram(curThread, histogram);
    }
    return result;
  }
public:
  explicit LoopMonitoringApp(int & argc, char ** argv);
  /// The event loop for a given thread has gotten stuck, or unstuck.
  /** A zero elapsed time indicates that the loop is not stuck. The signal will be
    * emitted periodically with increasing values of `elapsed` for a given thread as long
    * as the loop is stuck. The thread might not exist when this notification is received. */
  Q_SIGNAL void loopStateChanged(QThread *, int elapsed);
  /// The first event was received in a newly started thread's event loop.
  /** The thread might not exist when this notification is received. */
  Q_SIGNAL void newThread(QThread *, const QString & threadName);
  /// The thread has a new histogram available.
  /** This signal is not sent more often than each updatePeriod().
    * The thread might not exist when this notification is received. */
  Q_SIGNAL void newHistogram(QThread *, const LoopMonitoringApp::Histogram &);
  /// The thread has finished.
  /** The thread might not exist when this notification is received. A newHistogram
    * signal is always emitted prior to this signal's emission. */
  Q_SIGNAL void threadFinished(QThread *);
  /// The maximum number of milliseconds an event handler can run before the event loop
  /// is considered stuck.
  int timeout() const { return m_timeout; }
  Q_SLOT void setTimeout(int timeout) { m_timeout = timeout; }
  int updatePeriod() const { return m_updatePeriod; }
  Q_SLOT void setUpdatePeriod(int updatePeriod) { m_updatePeriod = updatePeriod; }
};
Q_DECLARE_METATYPE(QThread*)
Q_DECLARE_METATYPE(LoopMonitoringApp::Histogram)

LoopMonitoringApp::LoopMonitoringApp(int & argc, char ** argv) :
  LoopMonitoringApp::Base(argc, argv),
  m_timeout(1000), m_updatePeriod(250), m_notifier(this)
{
  qRegisterMetaType<QThread*>();
  qRegisterMetaType<LoopMonitoringApp::Histogram>();
  m_notifier.moveToThread(&m_notifierThread);
  m_notifierThread.start();
}

QImage renderHistogram(const LoopMonitoringApp::Histogram & h) {
  const int blockX = 2, blockY = 2;
  QImage img(1 + h.size() * blockX, 32 * blockY, QImage::Format_ARGB32_Premultiplied);
  img.fill(Qt::white);
  QPainter p(&img);
  int x = 0;
  for (auto it = h.begin(); it != h.end(); ++it) {
    qreal key = it.key() > 0 ? log2(it.key()) : 0.0;
    QBrush b = QColor::fromHsv(qRound(240.0*(1.0 - key/32.0)), 255, 255);
    p.fillRect(QRectF(x, img.height(), blockX, -log2(it.value()) * blockY), b);
    x += blockX;
  }
  return img;
}

class LoopMonitoringViewModel : public QStandardItemModel {
  Q_OBJECT
  QMap<QThread*, QPair<QStandardItem*, QStandardItem*>> m_threadItems;
  Q_SLOT void newThread(QThread * thread, const QString & threadName) {
    auto const caption = QString("0x%1 \"%2\"").arg(std::intptr_t(thread), 0, 16).arg(threadName);
    int row = rowCount() ? 1 : 0;
    insertRow(row);
    auto captionItem = new QStandardItem(caption);
    captionItem->setEditable(false);
    setItem(row, 0, captionItem);
    auto histogram = new QStandardItem;
    histogram->setEditable(false);
    setItem(row, 1, histogram);
    m_threadItems[thread] = qMakePair(captionItem, histogram);
    newHistogram(thread, LoopMonitoringApp::Histogram());
  }
  Q_SLOT void newHistogramImage(QThread * thread, const QImage & img) {
    auto it = m_threadItems.find(thread);
    if (it == m_threadItems.end()) return;
    it->second->setSizeHint(img.size());
    it->second->setData(img, Qt::DecorationRole);
  }
  Q_SIGNAL void newHistogramImageSignal(QThread * thread, const QImage & img);
  Q_SLOT void newHistogram(QThread * thread, const LoopMonitoringApp::Histogram & histogram) {
    QtConcurrent::run([this, thread, histogram]{
      emit newHistogramImageSignal(thread, renderHistogram(histogram));
    });
  }
  Q_SLOT void loopStateChanged(QThread * thread, int elapsed) {
    auto it = m_threadItems.find(thread);
    if (it == m_threadItems.end()) return;
    it->first->setData(elapsed ? QColor(Qt::red) : QColor(Qt::transparent), Qt::BackgroundColorRole);
  }
  Q_SLOT void threadFinished(QThread * thread) {
    auto it = m_threadItems.find(thread);
    if (it == m_threadItems.end()) return;
    it->first->setText(QString("Finished %1").arg(it->first->text()));
    m_threadItems.remove(thread);
  }
public:
  LoopMonitoringViewModel(QObject *parent = 0) : QStandardItemModel(parent) {
    connect(this, SIGNAL(newHistogramImageSignal(QThread*,QImage)),
            SLOT(newHistogramImage(QThread*,QImage)));
    LoopMonitoringApp * app = qobject_cast<LoopMonitoringApp*>(qApp);
    connect(app, SIGNAL(newThread(QThread*,QString)), SLOT(newThread(QThread*,QString)));
    connect(app, SIGNAL(newHistogram(QThread*,LoopMonitoringApp::Histogram)),
            SLOT(newHistogram(QThread*,LoopMonitoringApp::Histogram)));
    connect(app, SIGNAL(threadFinished(QThread*)), SLOT(threadFinished(QThread*)));
    connect(app, SIGNAL(loopStateChanged(QThread*,int)), SLOT(loopStateChanged(QThread*,int)));
  }
};

class WorkerObject : public QObject {
  Q_OBJECT
  int m_trials;
  double m_probability;
  QBasicTimer m_timer;
  void timerEvent(QTimerEvent * ev) {
    if (ev->timerId() != m_timer.timerId()) return;
    QThread::msleep(std::binomial_distribution<>(m_trials, m_probability)(reng));
  }
public:
  WorkerObject(QObject * parent = 0) : QObject(parent), m_trials(2000), m_probability(0.2) {}
  Q_SLOT void start() { m_timer.start(0, this); }
  int trials() const { return m_trials; }
  Q_SLOT void setTrials(int trials) { m_trials = trials; }
  double probability() const { return m_probability; }
  Q_SLOT void setProbability(double p) { m_probability = p; }
};

int main(int argc, char *argv[])
{
  LoopMonitoringApp app(argc, argv);
  LoopMonitoringViewModel model;
  WorkerObject workerObject;
  Thread workerThread;

  QWidget w;
  QGridLayout layout(&w);
  QTableView view;
  QLabel timeoutLabel;
  QSlider timeout(Qt::Horizontal);
  QGroupBox worker("Worker Thread");
  worker.setCheckable(true);
  worker.setChecked(false);
  QGridLayout wLayout(&worker);
  QLabel rangeLabel, probabilityLabel;
  QSlider range(Qt::Horizontal), probability(Qt::Horizontal);

  timeoutLabel.setMinimumWidth(50);
  timeoutLabel.connect(&timeout, SIGNAL(valueChanged(int)), SLOT(setNum(int))); // :(
  timeout.setMinimum(50);
  timeout.setMaximum(5000);
  timeout.setValue(app.timeout());
  view.setModel(&model);
  view.verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  layout.addWidget(&view, 0, 0, 1, 3);
  layout.addWidget(new QLabel("Timeout"), 1, 0);
  layout.addWidget(&timeoutLabel, 1, 1);
  layout.addWidget(&timeout, 1, 2);
  layout.addWidget(&worker, 2, 0, 1, 3);

  QObject::connect(&range, &QAbstractSlider::valueChanged, [&](int p){
    rangeLabel.setText(QString("Range %1 ms").arg(p));
    workerObject.setTrials(p);
  });
  QObject::connect(&probability, &QAbstractSlider::valueChanged, [&](int p){
    double prob = p / (double)probability.maximum();
    probabilityLabel.setText(QString("Probability %1").arg(prob, 0, 'g', 2));
    workerObject.setProbability(prob);
  });
  range.setMaximum(10000);
  range.setValue(workerObject.trials());
  probability.setValue(workerObject.probability() * probability.maximum());

  wLayout.addWidget(new QLabel("Sleep Time Binomial Distribution"), 0, 0, 1, 2);
  wLayout.addWidget(&rangeLabel, 1, 0);
  wLayout.addWidget(&range, 2, 0);
  wLayout.addWidget(&probabilityLabel, 1, 1);
  wLayout.addWidget(&probability, 2, 1);

  QObject::connect(&worker, &QGroupBox::toggled, [&](bool run) {
    if (run) {
      workerThread.start();
      QMetaObject::invokeMethod(&workerObject, "start");
    } else workerThread.quit();
  });
  QObject::connect(&timeout, &QAbstractSlider::valueChanged, &app, &LoopMonitoringApp::setTimeout);
  workerObject.moveToThread(&workerThread);
  w.show();
  return app.exec();
}

#include "main.moc"
