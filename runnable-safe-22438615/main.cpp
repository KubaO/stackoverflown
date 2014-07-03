#include <QThreadPool>
#include <QRunnable>
#include <QMutex>
#include <QDebug>

// This cannot be a class that can be derived from, since the destructor
// will be run *after* the derived class's destructor - thus it would not
// protect the derived class's members from premature destruction.
class RunnableWrapper Q_DECL_FINAL : public QRunnable {
   Q_DISABLE_COPY(RunnableWrapper)
   class LockerUnlocker {
      Q_DISABLE_COPY(LockerUnlocker)
      QMutexLocker * m_mutexLocker;
   public:
      explicit LockerUnlocker(QMutexLocker * m) : m_mutexLocker(m) {}
      ~LockerUnlocker() { m_mutexLocker->unlock(); }
   };
   QRunnable * m_wrapped;
   QMutex m_mutex;
   QMutexLocker m_lock;
   void run() Q_DECL_FINAL {
      LockerUnlocker unlocker(&m_lock);
      m_wrapped->run();
   }
public:
   explicit RunnableWrapper(QRunnable * r) : m_wrapped(r), m_lock(&m_mutex) {
      setAutoDelete(false);
   }
   void wait() { QMutexLocker lock(&m_mutex); }
   ~RunnableWrapper() {
      wait();
      if (m_wrapped->autoDelete()) delete m_wrapped;
   }
};

class Incrementer : public QRunnable {
   int * m_val;
   void run() Q_DECL_OVERRIDE { ++ *m_val; }
public:
   explicit Incrementer(int * val) : m_val(val) {}
   ~Incrementer() { qDebug() << __FUNCTION__; }
};

int main() {
   QThreadPool pool;
   int i = 0;
   {
      // Use with stack allocated runnable
      Incrementer inc(&i);
      inc.setAutoDelete(false); // Required without a wrapper as well!
      RunnableWrapper wrap(&inc);
      pool.start(&wrap);
   }
   qDebug() << i;
   {
      // Use with heap allocated runnable
      Incrementer * inc = new Incrementer(&i);
      RunnableWrapper wrap(inc);
      pool.start(&wrap);
   }
   qDebug() << i;
   return 0;
}
