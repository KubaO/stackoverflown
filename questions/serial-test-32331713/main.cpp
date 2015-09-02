#include <QtWidgets>

/// A thread that gives itself a bit of time to finish up, and then terminates.
class Thread : public QThread {
   Q_OBJECT
   Q_PROPERTY (int shutdownTimeout MEMBER m_shutdownTimeout)
   int m_shutdownTimeout { 1000 }; ///< in milliseconds
   QBasicTimer m_shutdownTimer;
   void timerEvent(QTimerEvent * ev) {
      if (ev->timerId() == m_shutdownTimer.timerId()) {
         if (! isFinished()) terminate();
      }
      QThread::timerEvent(ev);
   }
   bool event(QEvent *event) {
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
      Q_ASSERT(thread() != QThread::currentThread());
      stop();
      wait(50);
      terminate();
      wait();
   }
};

class LazyThread : public Thread {
   Q_OBJECT
   Q_PROPERTY(bool getStuck MEMBER m_getStuck)
   bool m_getStuck { false };
   void run() {
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
   bool eventFilter(QObject *, QEvent * ev) {
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

#include "main.moc"
