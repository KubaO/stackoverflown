// https://github.com/KubaO/stackoverflown/tree/master/questions/qbasictimer-stop-fix-50636079
#include <QtCore>

class Thread final : public QThread {
   Q_OBJECT
   void run() override {
      connect(QAbstractEventDispatcher::instance(this),
              &QAbstractEventDispatcher::aboutToBlock,
              this, &Thread::aboutToBlock);
      QThread::run();
   }
   QAtomicInt inDestructor;
public:
   using QThread::QThread;
   /// Take an object and prevent timer resource leaks when the object is about
   /// to become threadless.
   void takeObject(QObject *obj) {
      // Work around to prevent
      // QBasicTimer::stop: Failed. Possibly trying to stop from a different thread
      static constexpr char kRegistered[] = "__ThreadRegistered";
      static constexpr char kMoved[] = "__Moved";
      if (!obj->property(kRegistered).isValid()) {
         QObject::connect(this, &Thread::finished, obj, [this, obj]{
            if (!inDestructor.load() || obj->thread() != this)
               return;
            // The object is about to become threadless
            Q_ASSERT(obj->thread() == QThread::currentThread());
            obj->setProperty(kMoved, true);
            obj->moveToThread(this->thread());
         }, Qt::DirectConnection);
         QObject::connect(this, &QObject::destroyed, obj, [obj]{
            if (!obj->thread()) {
               obj->moveToThread(QThread::currentThread());
               obj->setProperty(kRegistered, {});
            }
            else if (obj->thread() == QThread::currentThread() && obj->property(kMoved).isValid()) {
               obj->setProperty(kMoved, {});
               QCoreApplication::sendPostedEvents(obj, QEvent::MetaCall);
            }
            else if (obj->thread()->eventDispatcher())
               QTimer::singleShot(0, obj, [obj]{ obj->setProperty(kRegistered, {}); });
         }, Qt::DirectConnection);

         obj->setProperty(kRegistered, true);
      }
      obj->moveToThread(this);
   }
   ~Thread() override {
      inDestructor.store(1);
      requestInterruption();
      quit();
      wait();
   }
   Q_SIGNAL void aboutToBlock();
};

int main(int argc, char *argv[]) {
   static_assert(QT_VERSION < QT_VERSION_CHECK(5,11,0), "");
   QCoreApplication app(argc, argv);
   QObject object1, object2;
   object1.startTimer(10);
   object2.startTimer(200);
   Thread workThread1, workThread2;
   QTimer::singleShot(500, &QCoreApplication::quit);
   workThread1.start();
   workThread2.start();
   workThread1.takeObject(&object1);
   workThread2.takeObject(&object2);
   app.exec();
}
#include "main.moc"






#if 0

class Thread;

struct TimerData {
   bool valid = false;
   QPointer<Thread> thread = nullptr;
   QList<QAbstractEventDispatcher::TimerInfo> timers;
};
Q_DECLARE_METATYPE(TimerData)

static constexpr char kTimers[] = "__Timers";

class Thread final : public QThread {
   Q_OBJECT
   bool inDestructor = false;
   void run() override {
      connect(QAbstractEventDispatcher::instance(this),
              &QAbstractEventDispatcher::aboutToBlock,
              this, &Thread::aboutToBlock);
      QThread::run();
   }
   static bool saveTimers(QObject *obj, Thread *thread) {
      auto dispatcher = QAbstractEventDispatcher::instance();
      if (!dispatcher)
         return false;
      qDebug() << "saving";
      const TimerData d{true, thread, dispatcher->registeredTimers(obj)};
      if (!dispatcher->unregisterTimers(obj))
         qDebug() << "failed";
      if (!d.timers.isEmpty())
         obj->setProperty(kTimers, QVariant::fromValue(d));
      qDebug() << "-" << d.timers.size() << d.thread;
      return true;
   }
   static TimerData getTimers(QObject *obj) {
      auto const vTimers = obj->property(kTimers);
      qDebug() << vTimers;
      if ((int)vTimers.type() != qMetaTypeId<TimerData>())
         return {};
      return vTimers.value<TimerData>();
   }
   static bool restoreTimers(QObject *obj, const TimerData &d) {
      qDebug() << "restore" << d.thread << d.timers.size();
      auto dispatcher = QAbstractEventDispatcher::instance();
      if (!dispatcher || !d.valid)
         return false;
      for (auto &timer : d.timers)
         dispatcher->registerTimer(timer.timerId, timer.interval, timer.timerType, obj);
      qDebug() << "restoring" << d.timers.size();
      obj->setProperty(kTimers, {});
      return true;
   }
public:
   using QThread::QThread;
   /// Take an object and prevent timer resource leaks when the object is about
   /// to become threadless.
   void takeObject(QObject *obj) {
      // Work around to prevent
      // QBasicTimer::stop: Failed. Possibly trying to stop from a different thread
      static constexpr char kRegistered[] = "__ThreadRegistered";
      if (!obj->property(kRegistered).isValid()) {
         QObject::connect(this, &Thread::finished, obj, [this, obj]{
            qDebug() << "+" << (QThread::currentThread() == this);
            if (!obj->thread() || obj->thread() != this)
               return;
            // The object was about to become threadless
            //saveTimers(obj, this);
            //obj->moveToThread(this->thread());
         }, Qt::DirectConnection);
         QObject::connect(this, &Thread::started, obj, [this, obj]{
            if (obj->thread() && obj->thread() != this->thread())
               return;
            auto d = getTimers(obj);
            if (d.thread != this)
               return;
            // The object was last on our thread and should be restored
            obj->moveToThread(this);
            qDebug() << "moving back";
            struct Event : QEvent {
               QObject *o;
               TimerData d;
               Event(QObject *o, TimerData &&d) : QEvent(QEvent::None), o(o), d(std::move(d)) {}
               ~Event() override {
                  qDebug() << "restored";
                  restoreTimers(o, d);
               }
            };
            QCoreApplication::postEvent(obj, new Event(obj, std::move(d)));
         });
         QObject::connect(this, &QObject::destroyed, obj, [this, obj]{
            qDebug() << "**" << obj << obj->thread() << this->thread() << QThread::currentThread();
            if (!obj->thread())
               obj->moveToThread(QThread::currentThread());
            if (!obj->thread() || obj->thread() == this->thread())
               restoreTimers(obj, getTimers(obj));
         }, Qt::DirectConnection);
         obj->setProperty(kRegistered, true);
      }
      obj->moveToThread(this);
   }
   ~Thread() override {
      requestInterruption();
      quit();
      wait();
   }
   Q_SIGNAL void aboutToBlock();
};

int main(int argc, char *argv[]) {
   static_assert(QT_VERSION < QT_VERSION_CHECK(5,11,0), "");
   QCoreApplication app(argc, argv);
   QObject object;
   object.startTimer(1000);
   object.startTimer(2000);
   qDebug() << QAbstractEventDispatcher::instance()->registeredTimers(&object).count();
   object.connect(&object, &QObject::destroyed, []{ qDebug() << "xxxxx"; });
   Thread workThread;
   workThread.start();
   workThread.takeObject(&object);
   QObject::connect(&workThread, &Thread::aboutToBlock, &QCoreApplication::quit);
   return app.exec();
}
#include "main.moc"
#endif
