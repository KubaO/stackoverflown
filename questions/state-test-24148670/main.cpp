// main.cpp
#include <QCoreApplication>
#include <QStateMachine>
#include <QEventLoop>
#include <QtTest>
#include <QTimer>

class Waiter {
   QTimer m_timer;
public:
   Waiter() {}
   Waiter(QObject * obj, const char * signal) {
      m_timer.connect(obj, signal, SIGNAL(timeout()));
   }
   void stop() {
      m_timer.stop();
      QMetaObject::invokeMethod(&m_timer, "timeout");
   }
   void wait(int timeout = 5000) {
      QEventLoop loop;
      m_timer.start(timeout);
      loop.connect(&m_timer, SIGNAL(timeout()), SLOT(quit()));
      loop.exec();
   }
};

class SignalWaiter : public QObject, public Waiter {
   Q_OBJECT
   int m_count;
   Q_SLOT void triggered() {
      ++ m_count;
      stop();
   }
public:
   SignalWaiter(QObject * obj, const char * signal) : m_count(0) {
      connect(obj, signal, SLOT(triggered()), Qt::QueuedConnection);
   }
   int count() const { return m_count; }
};

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
typedef QSignalSpy SignalSpy;
#else
class SignalSpy : public QSignalSpy, public Waiter {
public:
   SignalSpy(QObject * obj, const char * signal) :
      QSignalSpy(obj, signal), Waiter(obj, signal) {}
};
#endif

class Foo : public QObject {
   Q_OBJECT
   friend class FooTest;
   QStateMachine m_stateMachine;
   QState m_state1;
   QState m_state2;
   Q_SIGNAL void sigGoToStateOne();
   Q_SIGNAL void sigGoToStateTwo();
public:
   explicit Foo(QObject * parent = 0) :
      QObject(parent),
      m_state1(&m_stateMachine),
      m_state2(&m_stateMachine)
   {
      m_stateMachine.setInitialState(&m_state1);
      m_state1.addTransition(this, SIGNAL(sigGoToStateTwo()), &m_state2);
      m_state2.addTransition(this, SIGNAL(sigGoToStateOne()), &m_state1);
   }
   Q_SLOT void start() {
      m_stateMachine.start();
   }
};

class FooTest : public QObject {
   Q_OBJECT
   void call(QObject * obj, const char * method) {
      QMetaObject::invokeMethod(obj, method, Qt::QueuedConnection);
   }
   Q_SLOT void test1() {
      // Uses QSignalSpy
      Foo foo;
      SignalSpy state1(&foo.m_state1, SIGNAL(entered()));
      SignalSpy state2(&foo.m_state2, SIGNAL(entered()));
      call(&foo, "start");
      state1.wait();
      QCOMPARE(state1.count(), 1);
      call(&foo, "sigGoToStateTwo");
      state2.wait();
      QCOMPARE(state2.count(), 1);
      call(&foo, "sigGoToStateOne");
      state1.wait();
      QCOMPARE(state1.count(), 2);
   }

   Q_SLOT void test2() {
      // Uses SignalWaiter
      Foo foo;
      SignalWaiter state1(&foo.m_state1, SIGNAL(entered()));
      SignalWaiter state2(&foo.m_state2, SIGNAL(entered()));
      foo.start();
      state1.wait();
      QCOMPARE(state1.count(), 1);
      emit foo.sigGoToStateTwo();
      state2.wait();
      QCOMPARE(state2.count(), 1);
      emit foo.sigGoToStateOne();
      state1.wait();
      QCOMPARE(state1.count(), 2);
   }
};

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);
   FooTest test;
   QTest::qExec(&test, a.arguments());
   QMetaObject::invokeMethod(&a, "quit", Qt::QueuedConnection);
   return a.exec();
}

#include "main.moc"
