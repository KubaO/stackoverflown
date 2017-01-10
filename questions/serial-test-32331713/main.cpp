// https://github.com/KubaO/stackoverflown/tree/master/questions/serial-test-32331713
#include <QtWidgets>

/// A thread that gives itself a bit of time to finish up, and then terminates.
class Thread : public QThread {
   Q_OBJECT
   Q_PROPERTY (int shutdownTimeout MEMBER m_shutdownTimeout)
   int m_shutdownTimeout { 1000 }; ///< in milliseconds
   QBasicTimer m_shutdownTimer;
   void timerEvent(QTimerEvent * ev) override {
      if (ev->timerId() == m_shutdownTimer.timerId()) {
         if (! isFinished()) terminate();
      }
      QThread::timerEvent(ev);
   }
   bool event(QEvent *event) override {
      if (event->type() == QEvent::ThreadChange)
         QCoreApplication::postEvent(this, new QEvent(QEvent::None));
      else if (event->type() == QEvent::None && thread() == currentThread())
         // Hint that moveToThread(this) is an antipattern
         qWarning() << "The thread controller" << this << "is running in its own thread.";
      return QThread::event(event);
   }
   using QThread::requestInterruption; ///< Hidden, use stop() instead.
   using QThread::quit; ///< Hidden, use stop() instead.
public:
   Thread(QObject * parent = 0) : QThread(parent) {
      connect(this, &QThread::finished, this, [this]{ m_shutdownTimer.stop(); });
   }
   /// Indicates that the thread is attempting to finish.
   Q_SIGNAL void stopping();
   /// Signals the thread to stop in a general way.
   Q_SLOT void stop() {
      emit stopping();
      m_shutdownTimer.start(m_shutdownTimeout, this);
      requestInterruption(); // should break a run() that has no event loop
      quit();                // should break the event loop if there is one
   }
   ~Thread() {
      Q_ASSERT(!thread() || thread() == QThread::currentThread());
      stop();
      wait(50);
      if (isRunning()) terminate();
      wait();
   }
};

class LazyThread : public Thread {
   Q_OBJECT
   Q_PROPERTY(bool getStuck MEMBER m_getStuck)
   bool m_getStuck { false };
   void run() override {
      while (!isInterruptionRequested()) {
         msleep(100); // pretend that we're busy
      }
      qDebug() << "loop exited";
      if (m_getStuck) {
         qDebug() << "stuck";
         Q_FOREVER sleep(1);
      } else {
         qDebug() << "a little nap";
         sleep(2);
      }
   }
public:
   LazyThread(QObject * parent = 0) : Thread(parent) {
      setProperty("shutdownTimeout", 5000);
   }
};

class CloseThreadStopper : public QObject {
   Q_OBJECT
   QSet<Thread*> m_threads;
   void done(Thread* thread ){
      m_threads.remove(thread);
      if (m_threads.isEmpty()) emit canClose();
   }
   bool eventFilter(QObject *, QEvent * ev) override {
      if (ev->type() == QEvent::Close) {
         bool close = true;
         for (auto thread : m_threads) {
            if (thread->isRunning() && !thread->isFinished()) {
               close = false;
               ev->ignore();
               connect(thread, &QThread::finished, this, [this, thread]{ done(thread); });
               thread->stop();
            }
         }
         return !close;
      }
      return false;
   }
public:
   Q_SIGNAL void canClose();
   CloseThreadStopper(QObject * parent = 0) : QObject(parent) {}
   void addThread(Thread* thread) {
      m_threads.insert(thread);
      connect(thread, &QObject::destroyed, this, [this, thread]{ done(thread); });
   }
   void installOn(QWidget * w) {
      w->installEventFilter(this);
      connect(this, &CloseThreadStopper::canClose, w, &QWidget::close);
   }
};

int main(int argc, char *argv[])
{
   QApplication a { argc, argv };
   LazyThread thread;
   CloseThreadStopper stopper;
   stopper.addThread(&thread);

   struct Object : public QObject {
      ~Object() { qDebug() << __FUNCTION__ << "in" << thread(); }
   };
   Thread test;
   test.setObjectName("test");
   auto object = new Object;
   object->moveToThread(&test);
   test.start();
   QObject::connect(&test, &Thread::stopping, object, &QObject::deleteLater);
   QTimer::singleShot(1500, &test, &Thread::stop);

   QWidget ui;
   QGridLayout layout { &ui };
   QLabel state;
   QPushButton start { "Start" }, stop { "Stop" };
   QCheckBox stayStuck { "Keep the thread stuck" };
   layout.addWidget(&state, 0, 0, 1, 2);
   layout.addWidget(&stayStuck, 1, 0, 1, 2);
   layout.addWidget(&start, 2, 0);
   layout.addWidget(&stop, 2, 1);
   stopper.installOn(&ui);
   QObject::connect(&stayStuck, &QCheckBox::toggled, &thread, [&thread](bool v){
      thread.setProperty("getStuck", v);
   });

   QStateMachine sm;
   QState s_started { &sm }, s_stopping { &sm }, s_stopped { &sm };
   sm.setGlobalRestorePolicy(QState::RestoreProperties);
   s_started.assignProperty(&state, "text", "Running");
   s_started.assignProperty(&start, "enabled", false);
   s_stopping.assignProperty(&state, "text", "Stopping");
   s_stopping.assignProperty(&start, "enabled", false);
   s_stopping.assignProperty(&stop, "enabled", false);
   s_stopped.assignProperty(&state, "text", "Stopped");
   s_stopped.assignProperty(&stop, "enabled", false);

   for (auto state : { &s_started, &s_stopping })
      state->addTransition(&thread, SIGNAL(finished()), &s_stopped);
   s_started.addTransition(&thread, SIGNAL(stopping()), &s_stopping);
   s_stopped.addTransition(&thread, SIGNAL(started()), &s_started);
   QObject::connect(&start, &QPushButton::clicked, [&]{ thread.start(); });
   QObject::connect(&stop, &QPushButton::clicked, &thread, &Thread::stop);
   sm.setInitialState(&s_stopped);

   sm.start();
   ui.show();
   return a.exec();
}

//#include "main.moc"
#include <QSerialPort>

class CommThread : public Thread {
   Q_OBJECT
public:
   enum class Request { Disconnect };
private:
   QMutex m_mutex;
   QQueue<Request> m_requests;
   //...
   void run() override;
};

void CommThread::run()
{
   QString portname;
   QSerialPort port;

   port.setPortName(portname);
   port.setBaudRate(QSerialPort::Baud115200);

   if (!port.open(QIODevice::ReadWrite)){
      qWarning() << "Error opening Serial port within thread";
      return;
   }

   while (! isInterruptionRequested()) {
      QMutexLocker lock(&m_mutex);
      if (! m_requests.isEmpty()) {
         auto request = m_requests.dequeue();
         lock.unlock();
         if (request == Request::Disconnect) {
            qDebug() << "Entering disconnect sequence";
            QByteArray data;
            port.write(data);
            port.flush();
         }
         //...
      }
      lock.unlock();

      // The loop must run every 100ms to check for new requests
      if (port.waitForReadyRead(100)) {
         if (port.canReadLine()) {
            //...
         }
         QMutexLocker lock(&m_mutex);
         // Do something to a shared data structure
      }

      qDebug() << "The thread is exiting";
   }
}

//

namespace {
template <typename F>
static void postTo(QObject * obj, F && fun) {
   QObject signalSource;
   QObject::connect(&signalSource, &QObject::destroyed, obj, std::forward<F>(fun),
                    Qt::QueuedConnection);
}
}

class CommObject : public QObject {
   Q_OBJECT
   Q_PROPERTY(QImage image READ image NOTIFY imageChanged)
   mutable QMutex m_imageMutex;
   QImage m_image;
   QByteArray m_data;
   QString m_portName;
   QSerialPort m_port { this };
   void onData() {
      if (m_port.canReadLine()) {
         // process the line
      }
      QMutexLocker lock(&m_imageMutex);
      // Do something to the image
      emit imageChanged(m_image);
   }
public:
   /// Thread-safe
   Q_SLOT void disconnect() {
      postTo(this, [this]{
         qDebug() << "Entering disconnect sequence";
         m_port.write(m_data);
         m_port.flush();
      });
   }
   /// Thread-safe
   Q_SLOT void open() {
      postTo(this, [this]{
         m_port.setPortName(m_portName);
         m_port.setBaudRate(QSerialPort::Baud115200);
         if (!m_port.open(QIODevice::ReadWrite)){
            qWarning() << "Error opening the port";
            emit openFailed();
         } else {
            emit opened();
         }
      });
   }
   Q_SIGNAL void opened();
   Q_SIGNAL void openFailed();
   Q_SIGNAL void imageChanged(const QImage &);
   CommObject(QObject * parent = 0) : QObject(parent) {
      open();
      connect(&m_port, &QIODevice::readyRead, this, &CommObject::onData);
   }
   QImage image() const {
      QMutexLocker lock(&m_imageMutex);
      return m_image;
   }
};

#define main main1

int main(...) {
  //...
  Thread thread;
  thread.start();
  QScopedPointer<CommObject> comm(new CommObject);
  comm->moveToThread(&thread);
  QObject::connect(&thread, &Thread::stopping, comm.take(), &QObject::deleteLater);
  //...
}

#include "main.moc"
