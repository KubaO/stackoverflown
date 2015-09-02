#include <QtWidgets>

class WorkerBase : public QObject {
   Q_OBJECT
   Q_PROPERTY(bool active READ isActive WRITE setActive)
   QBasicTimer m_runTimer;
   bool m_active;
protected:
   void timerEvent(QTimerEvent * ev) {
      if (ev->timerId() != m_runTimer.timerId()) return;
      work();
   }
   virtual void workStarted() {}
   virtual void work() = 0;
   virtual void workEnded() {}
public:
   WorkerBase(QObject * parent = 0) : QObject(parent), m_active(false) {
      setActive(true);
   }
   /// This method is thread-safe.
   bool isActive() const { return m_active; }
   /// This method is thread-safe.
   void setActive(bool active) {
      QObject source;
      QObject::connect(&source, &QObject::destroyed, this, [this,active]{
         // The functor is executed in the thread context of this object
         if (m_active == active) return;
         if (active) {
            m_runTimer.start(0, this);
            workStarted();
         } else {
            m_runTimer.stop();
            workEnded();
         }
         m_active = active;
      }, thread() ? Qt::QueuedConnection : Qt::DirectConnection);
   }
   ~WorkerBase() {
      Q_ASSERT(QThread::currentThread() == thread() || !thread());
      setActive(false);
   }
};

class Worker : public WorkerBase {
   Q_OBJECT
   int m_runCount;
protected:
   void workStarted() Q_DECL_OVERRIDE {
      qDebug() << "Starting" << QThread::currentThread();
   }
   void work() Q_DECL_OVERRIDE {
      QThread::msleep(1000);
      ++ m_runCount;
      qDebug() << m_runCount << QThread::currentThread();
   }
   void workEnded() Q_DECL_OVERRIDE {
      qDebug() << "Finishing" << QThread::currentThread();
      emit finished();
   }
public:
   Worker(QObject * parent = 0) : WorkerBase(parent), m_runCount(0) {}
   Q_SIGNAL void finished();
};

class Thread : public QThread {
   using QThread::run; // final method
public:
   Thread(QObject * parent = 0) : QThread(parent) {}
   ~Thread() { quit(); wait(); }
};

class MainWindow : public QMainWindow {
   Q_OBJECT
   Worker m_worker;
   Thread m_workerThread;
   QLabel m_label;
protected:
   void closeEvent(QCloseEvent * ev) {
      if (m_worker.isActive()) {
         m_worker.setActive(false);
         ev->ignore();
      } else
         ev->accept();
   }
public:
   MainWindow(QWidget * parent = 0) : QMainWindow(parent),
      m_label("Hello :)\nClose the window to quit.")
   {
      setCentralWidget(&m_label);
      m_workerThread.setObjectName("m_worker");
      m_worker.moveToThread(&m_workerThread);
      connect(&m_worker, &Worker::finished, this, &QWidget::close);
      m_workerThread.start();
      qDebug() << "Main thread:" << QThread::currentThread();
   }
   ~MainWindow() {
      qDebug() << __FUNCTION__ << QThread::currentThread();
   }
};

int main(int argc, char ** argv)
{
   QApplication a(argc, argv);
   QThread::currentThread()->setObjectName("main");
   MainWindow w;
   w.show();
   w.setAttribute(Qt::WA_QuitOnClose);
   return a.exec();
}

#include "main.moc"
