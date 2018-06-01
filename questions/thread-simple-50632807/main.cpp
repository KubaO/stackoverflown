// https://github.com/KubaO/stackoverflown/tree/master/questions/thread-simple-50632807
#include <QtNetwork>

class Thread final : public QThread {
   Q_OBJECT
public:
   void takeObject(QObject *obj) {
      obj->moveToThread(this);
   }
   ~Thread() override {
      requestInterruption();
      quit();
      wait();
   }
};

class Class : public QObject {
   Q_OBJECT
   QBasicTimer m_workTimer;
   QNetworkAccessManager m_manager{this};
   void doWorkChunk() {
      qDebug() << "tick...";
      QThread::sleep(1); // emulate a blocking operation
   }
protected:
   void timerEvent(QTimerEvent *ev) override {
      if (ev->timerId() != m_workTimer.timerId())
         return;
      doWorkChunk();
   }
public:
   explicit Class(QObject *parent = {}) : QObject(parent) {
      m_workTimer.start(0, this);
   }
};

int main(int argc, char *argv[]) {
   QCoreApplication app(argc, argv);
   Class object;
   Thread workThread;
   workThread.start();
   workThread.takeObject(&object);
   QTimer::singleShot(3000, &QCoreApplication::quit);
   return app.exec();
}
#include "main.moc"
