#include <QCoreApplication>
#include <QThread>
#include <QEvent>
#include <QMap>
#include <QMutex>
#include <QDebug>
#include <functional>

class FunctorCallEvent : public QEvent {
   std::function<void()> m_fun;
public:
   FunctorCallEvent(const std::function<void()> & fun) :
      QEvent(QEvent::None), m_fun(fun) {}
   FunctorCallEvent(std::function<void()> && fun) :
      QEvent(QEvent::None), m_fun(fun) { qDebug() << "move semantics"; }
   void call() { m_fun(); }
};

class FunctorCallConsumer : public QObject {
   typedef QMap<QThread*, FunctorCallConsumer*> Map;
   static QObject * m_appThreadObject;
   static QMutex m_threadObjectMutex;
   static Map m_threadObjects;
   bool event(QEvent * ev) {
      if (!dynamic_cast<FunctorCallEvent*>(ev)) return QObject::event(ev);
      static_cast<FunctorCallEvent*>(ev)->call();
      return true;
   }
   FunctorCallConsumer() {}
   ~FunctorCallConsumer() {
      qDebug() << "consumer done for thread" << thread();
      Q_ASSERT(thread());
      QMutexLocker lock(&m_threadObjectMutex);
      m_threadObjects.remove(thread());
   }
   static void deleteAppThreadObject() {
      delete m_appThreadObject;
      m_appThreadObject = nullptr;
   }
public:
   static FunctorCallConsumer * forThread(QThread * thread) {
      QMutexLocker lock(&m_threadObjectMutex);
      Map map = m_threadObjects;
      lock.unlock();
      Map::const_iterator it = map.find(thread);
      if (it != map.end()) return *it;
      FunctorCallConsumer * consumer = new FunctorCallConsumer;
      consumer->moveToThread(thread);
      if (thread != qApp->thread())
         QObject::connect(thread, SIGNAL(finished()), consumer, SLOT(deleteLater()));
      lock.relock();
      it = m_threadObjects.find(thread);
      if (it == m_threadObjects.end()) {
         if (thread == qApp->thread()) {
            Q_ASSERT(! m_appThreadObject);
            m_appThreadObject = consumer;
            qAddPostRoutine(&deleteAppThreadObject);
         }
         m_threadObjects.insert(thread, consumer);
         return consumer;
      } else {
         delete consumer;
         return *it;
      }
   }
};

QObject * FunctorCallConsumer::m_appThreadObject = nullptr;
QMutex FunctorCallConsumer::m_threadObjectMutex;
FunctorCallConsumer::Map FunctorCallConsumer::m_threadObjects;

void postMetaCall(QThread * thread, const std::function<void()> & fun) {
   Q_ASSERT(thread);
   QCoreApplication::postEvent(FunctorCallConsumer::forThread(thread),
                               new FunctorCallEvent(fun));
}

void postMetaCall(QThread * thread, std::function<void()> && fun) {
   Q_ASSERT(thread);
   QCoreApplication::postEvent(FunctorCallConsumer::forThread(thread),
                               new FunctorCallEvent(std::forward<std::function<void()>>(fun)));
}

class Worker : public QThread {
   void run() {
      postMetaCall(qApp->thread(), []{
         qDebug() << "worker functor executes in thread" << QThread::currentThread();
      });
      QEventLoop loop;
      loop.processEvents();
   }
};

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);
   a.thread()->setObjectName("main");
   Worker worker;
   worker.setObjectName("worker");
   qDebug() << "worker thread:" << &worker;
   qDebug() << "main thread:" << QThread::currentThread();
   postMetaCall(&worker, []{ qDebug() << "main functor executes in thread" << QThread::currentThread(); });
   worker.start();
   worker.wait();
   QCoreApplication::processEvents();
   return 0;
}
