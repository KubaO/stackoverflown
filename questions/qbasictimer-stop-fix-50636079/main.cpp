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
public:
   void takeObject(QObject *obj) {
      // Work around to prevent
      // QBasicTimer::stop: Failed. Possibly trying to stop from a different thread
      QObject::connect(this, &Thread::finished, obj, [this, obj]{
         if (obj->thread() != this)
            return;
         // The object is about to become threadless
         obj->moveToThread(this->thread());
         QObject::connect(this, &QObject::destroyed, obj, [this, obj]{
            if (obj->thread() == this->thread())
               QCoreApplication::sendPostedEvents(obj, QEvent::MetaCall);
         });
      });
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
   Thread workThread;
   workThread.start();
   workThread.takeObject(&object);
   QObject::connect(&workThread, &Thread::aboutToBlock, &QCoreApplication::quit);
   return app.exec();
}
#include "main.moc"
