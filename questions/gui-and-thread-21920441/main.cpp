#include <QApplication>
#include <QPlainTextEdit>
#include <QThread>
#include <QBasicTimer>
#include <QTextStream>

//! A thread that's always safe to destruct.
class Thread : public QThread {
private:
   // This is a final class.
   using QThread::run;
public:
   Thread(QObject * parent = 0) : QThread(parent) {}
   ~Thread() {
      quit();
      wait();
   }
};

class IperfTester : public QObject {
   Q_OBJECT
   struct Test { int n; Test(int n_) : n(n_) {} };
   QList<Test> m_tests;
   QBasicTimer m_timer;
public:
   IperfTester(QObject * parent = 0) : QObject(parent) {
      for (int i = 0; i < 50; ++i) m_tests << Test(i+1);
   }
   //! Run the tests. This function is thread-safe.
   Q_SLOT void runTests() {
      QMetaObject::invokeMethod(this, "runTestsImpl");
   }
   Q_SIGNAL void message(const QString &);
private:
   Q_INVOKABLE void runTestsImpl() {
      m_timer.start(0, this);
   }
   void timerEvent(QTimerEvent * ev) {
      if (ev->timerId() != m_timer.timerId()) return;
      if (m_tests.isEmpty()) {
         m_timer.stop();
         return;
      }
      runTest(m_tests.first());
      m_tests.removeFirst();
   }
   void runTest(Test & test) {
      // do the work
      QString msg;
      QTextStream s(&msg);
      s << "Version:" << "3.11" << "\n";
      s << "Number:" << test.n << "\n";
      emit message(msg);
   }
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QPlainTextEdit log;
   // This order is important: the thread must be defined after the object
   // to be moved into the thread.
   IperfTester tester;
   Thread thread;
   tester.moveToThread(&thread);
   thread.start();
   log.connect(&tester, SIGNAL(message(QString)), SLOT(appendPlainText(QString)));
   log.show();
   tester.runTests();
   return a.exec();
   // Here, the thread is stopped and destructed first, following by a now threadless
   // tester. It would be an error if the tester object was destructed while its
   // thread existed (even if it was stopped!).
}

#include "main.moc"
