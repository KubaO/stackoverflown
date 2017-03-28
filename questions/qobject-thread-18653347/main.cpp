// https://github.com/KubaO/stackoverflown/tree/master/questions/qobject-thread-18653347
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

/// See http://stackoverflow.com/a/40382821/1329652
bool isSafe(QObject * obj) {
   Q_ASSERT(obj->thread() || qApp && qApp->thread() == QThread::currentThread());
   auto thread = obj->thread() ? obj->thread() : qApp->thread();
   return thread == QThread::currentThread();
}

class Helper : private QThread {
public:
   using QThread::usleep;
};

class Worker : public QObject {
   Q_OBJECT
   int m_counter;
   QBasicTimer m_timer;
   void timerEvent(QTimerEvent * ev) override;
public:
   Worker(QObject *parent = nullptr) : QObject(parent) {}
   /// This method is thread-safe.
   Q_SLOT void start() {
      if (!isSafe(this)) return (void)QMetaObject::invokeMethod(this, "start");
      if (m_timer.isActive()) return;
      m_counter = 0;
      m_timer.start(0, this);
   }
   /// This method is thread-safe.
   Q_INVOKABLE void startInThread(QObject *targetThread) {
      if (!isSafe(this)) return (void)QMetaObject::invokeMethod(this, "startInThread", Q_ARG(QObject*, targetThread));
      QObject::moveToThread(qobject_cast<QThread*>(targetThread));
      start();
   }
   Q_SIGNAL void done();
   Q_SIGNAL void progress(int percent, bool inGuiThread);
};

void Worker::timerEvent(QTimerEvent * ev)
{
   const int busyTime = 50; // [ms] - longest amount of time to stay busy
   const int testFactor = 128; // number of iterations between time tests
   const int maxCounter = 30000;
   if (ev->timerId() != m_timer.timerId()) return;

   const auto inGuiThread = []{ return QThread::currentThread() == qApp->thread(); };
   QElapsedTimer t;
   t.start();
   while (1) {
      // do some "work"
      Helper::usleep(100);
      m_counter ++;
      // exit when the work is done
      if (m_counter > maxCounter) {
         emit progress(100, inGuiThread());
         emit done();
         m_timer.stop();
         break;
      }
      // exit when we're done with a timed "chunk" of work
      // Note: QElapsedTimer::elapsed() may be expensive, so we call it once every testFactor iterations
      if ((m_counter % testFactor) == 0 && t.elapsed() > busyTime) {
         emit progress(m_counter*100/maxCounter, inGuiThread());
         break;
      }
   }
}

class Window : public QWidget {
   Q_OBJECT
   QVBoxLayout m_layout{this};
   QPushButton m_startGUI{"Start in GUI Thread"};
   QPushButton m_startWorker{"Start in Worker Thread"};
   QLabel m_label;
   QThread m_thread{this};
   Worker m_worker;

   Q_SLOT void showProgress(int p, bool inGuiThread) {
      m_label.setText(QString("%1 % in %2 thread")
                      .arg(p).arg(inGuiThread ? "gui" : "worker"));
   }
   Q_SLOT void on_startGUI_clicked() {
      m_worker.startInThread(qApp->thread());
   }
   Q_SLOT void on_startWorker_clicked() {
      m_worker.startInThread(&m_thread);
   }
public:
   Window(QWidget *parent = {}, Qt::WindowFlags f = {}) : QWidget(parent, f) {
      m_layout.addWidget(&m_startGUI);
      m_layout.addWidget(&m_startWorker);
      m_layout.addWidget(&m_label);
      m_thread.start();
      connect(&m_worker, SIGNAL(progress(int,bool)), SLOT(showProgress(int,bool)));
      connect(&m_startGUI, SIGNAL(clicked(bool)), SLOT(on_startGUI_clicked()));
      connect(&m_startWorker, SIGNAL(clicked(bool)), SLOT(on_startWorker_clicked()));
   }
   ~Window() {
      m_thread.quit();
      m_thread.wait();
   }
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   Window w;
   w.show();
   return a.exec();
}

#include "main.moc"
