//
// Qt 4/5 Solution Using a Custom Event and Consumer
//
#if 0
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
   FunctorCallEvent(const std::function<void()> & fun, QObject *) :
      QEvent(QEvent::None), m_fun(fun) {}
   FunctorCallEvent(std::function<void()> && fun, QObject *) :
      QEvent(QEvent::None), m_fun(fun) { qDebug() << "move semantics"; }
   void call() { m_fun(); }
};

#define HAS_FUNCTORCALLCONSUMER
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
   static bool needsRunningThread() { return false; }
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
// Common Code follows here
#endif

//
// Qt 4/5 Solution Using a Custom Event and Consumer for Main Thread Only
//
#if 0
#define STANDALONE
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
      QEvent(QEvent::None), m_fun(fun) {}
   void call() { m_fun(); }
};

class FunctorCallConsumer : public QObject {
   typedef QMap<QThread*, FunctorCallConsumer*> Map;
   static FunctorCallConsumer * m_appThreadObject;
   static QMutex m_threadObjectMutex;
   bool event(QEvent * ev) {
      if (!dynamic_cast<FunctorCallEvent*>(ev)) return QObject::event(ev);
      static_cast<FunctorCallEvent*>(ev)->call();
      return true;
   }
   FunctorCallConsumer() {}
   ~FunctorCallConsumer() {}
   static void deleteAppThreadObject() {
      delete m_appThreadObject;
      m_appThreadObject = nullptr;
   }
public:
   static FunctorCallConsumer * instance() {
      QMutexLocker lock(&m_threadObjectMutex);
      if (! m_appThreadObject) {
            m_appThreadObject = new FunctorCallConsumer;
            qAddPostRoutine(&deleteAppThreadObject);
         }
      return m_appThreadObject;
   }
};

FunctorCallConsumer * FunctorCallConsumer::m_appThreadObject = nullptr;
QMutex FunctorCallConsumer::m_threadObjectMutex;

void postToMainThread(const std::function<void()> & fun) {
   QCoreApplication::postEvent(FunctorCallConsumer::instance(), new FunctorCallEvent(fun));
}

void postToMainThread(std::function<void()> && fun) {
   QCoreApplication::postEvent(FunctorCallConsumer::instance(),
                               new FunctorCallEvent(std::move(fun)));
}

class Worker : public QThread {
   void run() {
      postToMainThread([]{
         qDebug() << "worker functor executes in thread" << QThread::currentThread();
      });
   }
public:
   ~Worker() { quit(); wait(); }
};

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);
   a.thread()->setObjectName("main");
   Worker worker;
   worker.setObjectName("worker");
   qDebug() << "worker thread:" << &worker;
   qDebug() << "main thread:" << QThread::currentThread();
   worker.start();
   a.connect(&worker, SIGNAL(finished()), SLOT(quit()));
   return a.exec();
}

#endif

//
// Qt 4/5 Solution Using QEvent Destructor
//
#if 0
#include <QCoreApplication>
#include <QThread>
#include <QEvent>
#include <QAbstractEventDispatcher>
#include <QDebug>
#include <functional>

class FunctorCallEvent : public QEvent {
   std::function<void()> m_fun;
   QThread * m_thread;
public:
   FunctorCallEvent(const std::function<void()> & fun, QObject * receiver) :
      QEvent(QEvent::None), m_fun(fun), m_thread(receiver->thread()) {}
   FunctorCallEvent(std::function<void()> && fun, QObject * receiver) :
      QEvent(QEvent::None), m_fun(std::move(fun)), m_thread(receiver->thread()) { qDebug() << "move semantics"; }
   ~FunctorCallEvent() {
      if (QThread::currentThread() == m_thread)
         m_fun();
      else
         qWarning() << "Dropping a functor call destined for thread" << m_thread;
   }
};
// Common Code follows here
#endif

//
// Qt 5 Solution Using the Private QMetaCallEvent
//
#if 0
#include <QCoreApplication>
#include <QThread>
#include <QAbstractEventDispatcher>
#include <QDebug>
#include <private/qobject_p.h>
#include <functional>

class FunctorCallEvent : public QMetaCallEvent {
public:
   template <typename Functor>
   FunctorCallEvent(const Functor & fun, QObject * receiver) :
      QMetaCallEvent(new QtPrivate::QFunctorSlotObject<Functor, 0, typename QtPrivate::List_Left<void, 0>::Value, void>(fun),
                     receiver, 0, 0, 0, (void**)malloc(sizeof(void*))) {}
   // Metacalls with slot objects require an argument array for the return type, even if it's void.
};
// Common Code follows here
#endif

//
// Qt 5 Solution Using a Temporary Object as The Signal Source
//
#if 1
#include <QCoreApplication>
#include <QThread>
#include <QAbstractEventDispatcher>
#include <QDebug>
#include <functional>

namespace FunctorCallConsumer { QObject * forThread(QThread*); }

#define HAS_POSTMETACALL
void postMetaCall(QThread * thread, const std::function<void()> & fun) {
   QObject signalSource;
   QObject::connect(&signalSource, &QObject::destroyed,
                    FunctorCallConsumer::forThread(thread), [=](QObject*){ fun(); });
}
// Common Code follows here
#endif

//
// Common Code
//
#if !defined(STANDALONE)

#ifndef HAS_FUNCTORCALLCONSUMER
namespace FunctorCallConsumer {
bool needsRunningThread() { return true; }
QObject * forThread(QThread * thread) {
   Q_ASSERT(thread);
   QObject * target = thread == qApp->thread()
         ? static_cast<QObject*>(qApp) : QAbstractEventDispatcher::instance(thread);
   Q_ASSERT_X(target, "postMetaCall", "the receiver thread must have an event loop");
   return target;
}
}
#endif

#ifndef HAS_POSTMETACALL
void postMetaCall(QThread * thread, const std::function<void()> & fun) {
   auto receiver = FunctorCallConsumer::forThread(thread);
   QCoreApplication::postEvent(receiver, new FunctorCallEvent(fun, receiver));
}

void postMetaCall(QThread * thread, std::function<void()> && fun) {
   auto receiver = FunctorCallConsumer::forThread(thread);
   QCoreApplication::postEvent(receiver,
                               new FunctorCallEvent(std::move(fun), receiver));
}
#endif

class Worker : public QThread {
   QMutex m_started;
   void run() {
      m_started.unlock();
      postMetaCall(qApp->thread(), []{
         qDebug() << "worker functor executes in thread" << QThread::currentThread();
      });
      QThread::run();
   }
public:
   Worker(QObject * parent = 0) : QThread(parent) { m_started.lock(); }
   ~Worker() { quit(); wait(); }
   void waitForStart() { m_started.lock(); m_started.unlock(); }
};

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);
   a.thread()->setObjectName("main");
   Worker worker;
   worker.setObjectName("worker");
   qDebug() << "worker thread:" << &worker;
   qDebug() << "main thread:" << QThread::currentThread();
   if (FunctorCallConsumer::needsRunningThread()) {
      worker.start();
      worker.waitForStart();
   }
   postMetaCall(&worker, []{ qDebug() << "main functor executes in thread" << QThread::currentThread(); });
   if (!FunctorCallConsumer::needsRunningThread()) worker.start();
   QMetaObject::invokeMethod(&a, "quit", Qt::QueuedConnection);
   return a.exec();
}

#endif
