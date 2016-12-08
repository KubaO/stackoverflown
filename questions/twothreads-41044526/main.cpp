// https://github.com/KubaO/stackoverflown/tree/master/questions/twothreads-41044526
#include <QtCore>

// see http://stackoverflow.com/questions/40382820/how-to-leverage-qt-to-make-a-qobject-method-thread-safe
template <typename Fun> void safe(QObject * obj, Fun && fun) {
    Q_ASSERT(obj->thread() || qApp && qApp->thread() == QThread::currentThread());
    if (Q_LIKELY(obj->thread() == QThread::currentThread()))
        return fun();
    struct Event : public QEvent {
      Fun fun;
      Event(Fun && fun) : QEvent(QEvent::None), fun(std::move(fun)) {}
      ~Event() { fun(); }
    };
    QCoreApplication::postEvent(obj->thread() ? obj : qApp, new Event(std::move(fun)));
}

class Worker : public QObject {
   Q_OBJECT
   QBasicTimer m_timer;
   int n = 0;
   void timerEvent(QTimerEvent *event) override {
      if (event->timerId() == m_timer.timerId())
         emit hasData(n++);
   }
public:
   Q_SIGNAL void hasData(int);
   Q_SLOT void onData(int d) { qDebug() << QThread::currentThread() << "got data" << d; }
   void start() {
      safe(this, [this]{ m_timer.start(50,this); });
   }
   void quit() {
      safe(this, [this]{ m_timer.stop(); thread()->quit(); });
   }
};

class Library {
   QByteArray dummy{"dummy"};
   int argc = 1;
   char *argv[2] = {dummy.data(), nullptr};
   QScopedPointer<QCoreApplication> app;
   static Library *m_self;
   struct {
      Worker worker;
      QThread thread;
   } m_jobs[3];
public:
   Library() {
      Q_ASSERT(!instance());
      m_self = this;
      if (!qApp) app.reset(new QCoreApplication(argc, argv));

      for (auto &job : m_jobs) {
         job.worker.moveToThread(&job.thread);
         job.thread.start();
         job.worker.start();
         QObject::connect(&job.worker, &Worker::hasData, &m_jobs[0].worker, &Worker::onData);
      }
   }
   ~Library() {
      for (auto &job : m_jobs) {
         job.worker.quit();
         job.thread.wait();
      }
   }
   static Library *instance() { return m_self; }
};
Library *Library::m_self;

// API
void initLib() {
   new Library;
}

void finishLib() {
   delete Library::instance();
}

int main()
{
   initLib();
   QThread::sleep(3);
   finishLib();
}

#include "main.moc"
