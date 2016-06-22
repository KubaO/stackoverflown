// https://github.com/KubaO/stackoverflown/tree/master/questions/worker-shared-37956073
#if 0
#include <QtCore>

struct DataClass : public QObject {
   DataClass() { qDebug() << __FUNCTION__; }
   ~DataClass() { qDebug() << __FUNCTION__; }
};
void doSomeReallyLongUninterruptibleWork(DataClass*) { QThread::sleep(2); }

class WorkerThread : public QThread {
   Q_OBJECT
public:
   void run() override {
      auto result = new DataClass;
      doSomeReallyLongUninterruptibleWork(result);
      result->moveToThread(qApp->thread());
      emit workComplete(result);
      QObject::connect(this, &QThread::finished, result, [result]{
         if (!result->parent()) {
            qDebug() << "DataClass is unclaimed and will deleteLater";
            result->deleteLater();
         }
      });
   }
   Q_SIGNAL void workComplete(DataClass*);
};

class MyWidget : public QObject {
   void processData(DataClass * result) {
      // Do stuff with result
      // Retain ownership (optional)
      if (true) result->setParent(this);
   }
public:
   void doBlockingWork() {
      auto worker = new WorkerThread;
      connect(worker, &WorkerThread::workComplete, this, &MyWidget::processData);
      connect(worker, &WorkerThread::finished, worker, &WorkerThread::deleteLater);
      worker->start();
   }
   ~MyWidget() { qDebug() << __FUNCTION__; }
};

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   QScopedPointer<MyWidget> w{new MyWidget};
   w->doBlockingWork();
   QTimer::singleShot(1000, w.data(), [&]{ w.reset(); });
   QTimer::singleShot(3000, qApp, &QCoreApplication::quit);
   return app.exec();
}

#include "main.moc"
#endif

#if 1
#include <QtCore>
#include <QtConcurrent>

struct DataClass : public QObject {
   DataClass() { qDebug() << __FUNCTION__; }
   ~DataClass() { qDebug() << __FUNCTION__; }
   Q_SIGNAL void ready();
   Q_OBJECT
};
void doSomeReallyLongUninterruptibleWork(DataClass*) { QThread::sleep(2); }

// Let's not pollute the default pool with long-running stuff
Q_GLOBAL_STATIC(QThreadPool, longPool)

class MyWidget : public QObject {
   Q_OBJECT
   void processData(DataClass * result) {
      // Do stuff with result
      // Retain ownership (optional)
      if (true) result->setParent(this);
   }
   Q_SIGNAL void foo();
public:
   void doBlockingWork() {
      auto result = new DataClass;
      connect(result, &DataClass::ready, this, [=]{ MyWidget::processData(result); });
      result->moveToThread(nullptr);
      QtConcurrent::run(longPool, [result]{
         result->moveToThread(QThread::currentThread());
         doSomeReallyLongUninterruptibleWork(result);
         result->moveToThread(qApp->thread());
         emit result->ready();
         QTimer::singleShot(0, result, [result]{
            if (!result->parent()) {
               qDebug() << "DataClass is unclaimed and will deleteLater";
               result->deleteLater();
            }
         });
      });
   }
   ~MyWidget() { qDebug() << __FUNCTION__; }
};

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   QScopedPointer<MyWidget> w{new MyWidget};
   w->doBlockingWork();
   QTimer::singleShot(1000, w.data(), [&]{ w.reset(); });
   QTimer::singleShot(3000, qApp, &QCoreApplication::quit);
   return app.exec();
}

#include "main.moc"

#endif
