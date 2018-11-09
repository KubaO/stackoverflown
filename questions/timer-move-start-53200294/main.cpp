// https://github.com/KubaO/stackoverflown/tree/master/questions/label-font-18896933timer-move-start-53200294
#include <QtCore>
int main(int argc, char *argv[]) {
   QCoreApplication app(argc, argv);
   int fired = 0;

   QTimer timer;
   QThread connectionThread;
   QObject::connect(&timer, &QTimer::timeout, [&] {
      Q_ASSERT(QThread::currentThread() == &connectionThread);
      ++fired;
      timer.moveToThread(qApp->thread());  // move the timer back to allow destruction
      QCoreApplication::quit();
   });
   timer.start(3000);
   timer.moveToThread(&connectionThread);

   Q_ASSERT(!fired);
   connectionThread.start();
   app.exec();
   Q_ASSERT(fired);

   connectionThread.quit();
   connectionThread.wait();
}
