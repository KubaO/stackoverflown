#include <QThread>
#include <QPointer>
#include <QCoreApplication>

class Thread : public QThread {
   using QThread::run; // final
public:
   Thread(QObject * parent = 0) : QThread(parent) {}
   ~Thread() { quit(); wait(); }
};

class ThreadQuitter {
public:
   typedef QList<QPointer<Thread>> List;
private:
   List m_threads;
   Q_DISABLE_COPY(ThreadQuitter)
public:
   ThreadQuitter() {}
   ThreadQuitter(const List & threads) : m_threads(threads) {}
   ThreadQuitter(List && threads) : m_threads(std::move(threads)) {}
   ThreadQuitter & operator<<(Thread* thread) { m_threads << thread; return *this; }
   ThreadQuitter & operator<<(Thread& thread) { m_threads << &thread; return *this; }
   ~ThreadQuitter() {
      foreach(Thread* thread, m_threads) thread->quit();
   }
};

int main(int argc, char ** argv) {
   QCoreApplication app(argc, argv);
   QObject worker1, worker2;
   Thread thread1, thread2;
   // Style 1
   ThreadQuitter quitter;
   quitter << thread1 << thread2;
   // Style 2
   ThreadQuitter quitterB(ThreadQuitter::List() << &thread1 << &thread2);
   //
   worker1.moveToThread(&thread1);
   worker2.moveToThread(&thread2);
   thread1.start();
   thread2.start();

   QMetaObject::invokeMethod(&app, "quit", Qt::QueuedConnection);
   return app.exec();
}
