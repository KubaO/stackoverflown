// https://github.com/KubaO/stackoverflown/tree/master/questions/thread-multiobject-40645483
#include <QtCore>

class Worker : public QObject {
   Q_OBJECT
   QBasicTimer m_timer;
   int n = 0;
   void timerEvent(QTimerEvent * ev) override {
      if (ev->timerId() == m_timer.timerId()) {
         qDebug() << this << n++;
      }
   }
   Q_SIGNAL void waitSignal();
public:
   Worker() {
      m_timer.start(0, this);
      connect(this, &Worker::stop, this, [=]{ m_timer.stop(); });
      connect(this, &Worker::waitSignal, this, []{}, Qt::BlockingQueuedConnection);
   }
   Q_SIGNAL void stop();
   void wait() {
      if (QThread::currentThread() != thread()) emit waitSignal();
   }
   ~Worker() { stop(); wait(); }
};

struct Thread : public QThread {
   ~Thread() { quit(); wait(); }
};

int main(int argc, char **argv) {
   QCoreApplication app{argc, argv};
   Thread thread;
   Worker workers[3];
   thread.start();
   for (auto & worker : workers)
      worker.moveToThread(&thread);
   QTimer::singleShot(100, []{ qApp->quit(); });
   return app.exec();
}
#include "main.moc"
