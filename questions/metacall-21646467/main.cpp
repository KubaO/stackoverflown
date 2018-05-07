// https://github.com/KubaO/stackoverflown/tree/master/questions/metacall-21646467
#define SOL0 // 0..6

#ifdef SOL0
#define STANDALONE
#include <QtCore>
#include <type_traits>

// Qt 5/4 - preferred, has least allocations

namespace detail {
template <typename F>
struct FEvent : QEvent {
   using Fun = typename std::decay<F>::type;
   const QObject *const obj;
   const QMetaObject *const type = obj->metaObject();
   Fun fun;
   template <typename Fun>
   FEvent(const QObject *obj, Fun &&fun) : QEvent(QEvent::None), obj(obj), fun(std::forward<Fun>(fun)) {}
   ~FEvent() {
      if (obj->metaObject()->inherits(type)) // ensure that the object is not being destructed
         fun();
   }
}; }

template <typename F>
static void postToObject(F &&fun, QObject *obj = qApp) {
   if (qobject_cast<QThread*>(obj))
      qWarning() << "posting a call to a thread object - consider using postToThread";
   QCoreApplication::postEvent(obj, new detail::FEvent<F>(obj, std::forward<F>(fun)));
}

template <typename F>
static void postToThread(F &&fun, QThread *thread = qApp->thread()) {
   QObject * obj = QAbstractEventDispatcher::instance(thread);
   Q_ASSERT(obj);
   QCoreApplication::postEvent(obj, new detail::FEvent<F>(obj, std::forward<F>(fun)));
}

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
// Qt 5 - alternative version

template <typename F>
static void postToObject2(F &&fun, QObject *obj = qApp) {
   if (qobject_cast<QThread*>(obj))
      qWarning() << "posting a call to a thread object - consider using postToThread";
   QObject src;
   QObject::connect(&src, &QObject::destroyed, obj,
                    [f = fun, type = obj->metaObject(), obj]
   {
      if (obj->metaObject()->inherits(type))
         f();
   }, Qt::QueuedConnection);
}

template <typename F>
static void postToThread2(F &&fun, QThread *thread = qApp->thread()) {
   QObject * obj = QAbstractEventDispatcher::instance(thread);
   Q_ASSERT(obj);
   QObject src;
   QObject::connect(&src, &QObject::destroyed, obj,
                    [f = std::forward<F>(fun), type = obj->metaObject(), obj]
   {
      if (obj->metaObject()->inherits(type))
         f();
   }, Qt::QueuedConnection);
}

void test0() {
   QThread t;
   QObject o;
   o.moveToThread(&t);

   // Execute in given object's thread
   postToObject2([&]{ o.setObjectName("hello"); }, &o);
   // or
   //postToObject2(std::bind(&QObject::setObjectName, &o, "hello"), &o);

   // Execute in given thread
   postToThread2([]{ qDebug() << "hello from worker thread"; });

   // Execute in the main thread
   postToThread2([]{ qDebug() << "hello from main thread"; });
}
#endif

void test1() {
   QThread t;
   QObject o;
   o.moveToThread(&t);

   // Execute in given object's thread
   postToObject([&]{ o.setObjectName("hello"); }, &o);
   // or
   //postToObject(std::bind(&QObject::setObjectName, &o, "hello"), &o);

   // Execute in given thread
   postToThread([]{ qDebug() << "hello from worker thread"; });

   // Execute in the main thread
   postToThread([]{ qDebug() << "hello from main thread"; });
}

// Qt 5/4

template <typename T, typename R>
static void postToObject(T * obj, R(T::* method)()) {
   struct Event : public QEvent {
      T * obj;
      R(T::* method)();
      Event(T * obj, R(T::*method)()):
         QEvent(QEvent::None), obj(obj), method(method) {}
      ~Event() { (obj->*method)(); }
   };
   if (qobject_cast<QThread*>(obj))
      qWarning() << "posting a call to a thread object - this may be a bug";
   QCoreApplication::postEvent(obj, new Event(obj, method));
}


void test2() {
   QThread t;
   struct MyObject : QObject { void method() {} } obj;
   obj.moveToThread(&t);

   // Execute in obj's thread
   postToObject(&obj, &MyObject::method);
}

int main() {}

#endif

//
// Qt 4/5 Solution Using a Custom Event and Consumer
//
#ifdef SOL1
#include <QtCore>
#include <functional>

class FunctorCallEvent : public QEvent {
   std::function<void()> m_fun;
public:
   FunctorCallEvent(const std::function<void()> & fun, QObject *) :
      QEvent(QEvent::None), m_fun(fun) {}
   FunctorCallEvent(std::function<void()> && fun, QObject *) :
      QEvent(QEvent::None), m_fun(std::move(fun)) { qDebug() << "move semantics"; }
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

QObject * FunctorCallConsumer::m_appThreadObject;
QMutex FunctorCallConsumer::m_threadObjectMutex;
FunctorCallConsumer::Map FunctorCallConsumer::m_threadObjects;
// Common Code follows here
#endif

//
// Qt 4/5 Solution Using a Custom Event and Consumer for Main Thread Only
//
#ifdef SOL2
#define STANDALONE
#include <QtCore>
#include <functional>

class FunctorCallEvent : public QEvent {
   std::function<void()> m_fun;
public:
   FunctorCallEvent(const std::function<void()> & fun) :
      QEvent(QEvent::None), m_fun(fun) {}
   FunctorCallEvent(std::function<void()> && fun) :
      QEvent(QEvent::None), m_fun(std::move(fun)) {}
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
         m_appThreadObject->moveToThread(qApp->thread());
         qAddPostRoutine(&deleteAppThreadObject);
      }
      return m_appThreadObject;
   }
};

FunctorCallConsumer * FunctorCallConsumer::m_appThreadObject;
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
#ifdef SOL3
#include <QtCore>
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
#ifdef SOL4
#include <QtCore>
#include <private/qobject_p.h>
#include <functional>

class FunctorCallEvent : public QMetaCallEvent {
public:
   template <typename Functor>
   FunctorCallEvent(Functor && fun, QObject * receiver) :
      QMetaCallEvent(new QtPrivate::QFunctorSlotObject<Functor, 0, typename QtPrivate::List_Left<void, 0>::Value, void>
                     (std::forward<Functor>(fun)), receiver, 0, 0, 0, (void**)malloc(sizeof(void*))) {}
   // Metacalls with slot objects require an argument array for the return type, even if it's void.
};
// Common Code follows here
#endif

//
// Qt 5 Solution Using a Temporary Object as The Signal Source
//
#ifdef SOL5
#include <QtCore>
#include <functional>

namespace FunctorCallConsumer { QObject * forThread(QThread*); }

#define HAS_POSTMETACALL
void postMetaCall(QThread * thread, const std::function<void()> & fun) {
   QObject signalSource;
   QObject::connect(&signalSource, &QObject::destroyed,
                    FunctorCallConsumer::forThread(thread), [=](QObject*){ fun(); });
}
#ifdef __cpp_init_captures
void postMetaCall(QThread * thread, std::function<void()> && fun) {
   QObject signalSource;
   QObject::connect(&signalSource, &QObject::destroyed,
                    FunctorCallConsumer::forThread(thread), [fun(std::move(fun))](QObject*){ fun(); });
}
#endif
// Common Code follows here
#endif

//
// Qt 5 Solution Using a Temporary Object as The Signal Source for the Main Thread Only
//
#ifdef SOL6
#define STANDALONE
#include <QtCore>
#include <functional>

void postToMainThread(const std::function<void()> & fun) {
   QObject signalSource;
   QObject::connect(&signalSource, &QObject::destroyed, qApp, [=](QObject*){
      fun();
   });
}

#ifdef __cpp_init_captures
void postToMainThread(std::function<void()> && fun) {
   QObject signalSource;
   QObject::connect(&signalSource, &QObject::destroyed, qApp, [fun(std::move(fun))](QObject*){
      fun();
   });
}
#endif

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
