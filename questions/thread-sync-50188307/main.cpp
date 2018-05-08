// https://github.com/KubaO/stackoverflown/tree/master/questions/thread-sync-50188307
#include <QtCore>

// see https://stackoverflow.com/a/21653558/1329652
namespace detail { template <typename F> struct FEvent : QEvent {
   const QObject *const obj;
   const QMetaObject *const type = obj->metaObject();
   typename std::decay<F>::type fun;
   template <typename Fun>
   FEvent(const QObject *obj, Fun &&fun) :
      QEvent(QEvent::None), obj(obj), fun(std::forward<Fun>(fun)) {}
   ~FEvent() { // ensure that the object is not being destructed
      if (obj->metaObject()->inherits(type)) fun();
   }
}; }

template <typename F> static void post(QObject *obj, F &&fun) {
   Q_ASSERT(!qobject_cast<QThread*>(obj));
   QCoreApplication::postEvent(obj, new detail::FEvent<F>(obj, std::forward<F>(fun)));
}

class Thread final : public QThread {
   Q_OBJECT
   void run() override {
      if (!eventDispatcher())
         QThread::run();
   }
public:
   using QThread::QThread;
   using QThread::exec;
   ~Thread() override {
      requestInterruption();
      quit();
      wait();
   }
   template <typename F> void on_start(F &&fun) {
      connect(this, &QThread::started, std::forward<F>(fun));
   }
};

#include <memory>

using app_logger = QObject;
using room_logger_manager = QObject;

class io_manager : public QObject {
   Q_OBJECT
   app_logger app_log{this};
   room_logger_manager room_log_mgr{this};
public:
   using QObject::QObject;
};

class network_manager : public QObject {
   Q_OBJECT
public:
   network_manager(QObject *parent = {}) : QObject(parent) {
      do_something1();
      do_something2();
   }
   void do_something1() { qDebug() << __FUNCTION__; }
   void do_something2() { qDebug() << __FUNCTION__; qApp->quit(); }
};

int main(int argc, char *argv[]) {
   QCoreApplication app{argc, argv};
   QPointer<io_manager> iomgr; //optional
   QPointer<network_manager> netmgr; //optional
   Thread io_thread, network_thread;
   io_thread.on_start([&]{
      qDebug() << "I/O thread is running.";
      io_manager mgr;
      iomgr = &mgr;
      network_thread.start();
      io_thread.exec();
   });
   network_thread.on_start([&]{
      qDebug() << "Network thread is running.";
      network_manager mgr;
      netmgr = &mgr;
      network_thread.exec();
   });
   io_thread.start();
   return app.exec(); // RAII all the way!
}
#include "main.moc"
