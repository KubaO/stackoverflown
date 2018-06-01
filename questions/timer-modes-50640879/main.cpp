// https://github.com/KubaO/stackoverflown/tree/master/questions/timer-modes-50640879
#include <QtWidgets>

struct Thread final : QThread {
   ~Thread() override { finish(); wait(); }
   void finish() { quit(); requestInterruption(); }
};

int main(int argc, char *argv[]) {
   QApplication app(argc, argv);
   QPushButton toggle{"Click to Stop"};
   toggle.setMinimumSize(300, 150);
   toggle.show();

   Thread thread;
   QTimer timer;
   QObject::connect(&toggle, &QPushButton::clicked, [&]{
      thread.finish();
      toggle.setDisabled(true);
   });
   QObject::connect(&thread, &Thread::finished, &toggle, &QWidget::close);

   constexpr enum { DriftySleep, CompensatedSleep, Timer } mode = CompensatedSleep;
   qint64 constexpr setPeriod = 7000;

   auto dump = [period = Q_INT64_C(0), watch = QElapsedTimer()](qint64 load = {}) mutable {
      qint64 current = watch.isValid() ? watch.elapsed() : 0;
      auto dateTime = QDateTime::currentDateTime();
      qDebug() << dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz") << current;
      if (!watch.isValid()) {
         watch.start();
         period = load;
      } else
         period = watch.restart();
      return period;
   };
   if (mode != Timer) QObject::connect(&thread, &Thread::started, [&]{
      auto period = dump(setPeriod);
      while (!thread.isInterruptionRequested()) {
         QThread::msleep(setPeriod*2 - ((mode == CompensatedSleep) ? period : setPeriod));
         period = dump();
      }
   });
   else {
      timer.setTimerType(Qt::PreciseTimer);
      timer.start(setPeriod);
      timer.moveToThread(&thread);
      QObject::connect(&thread, &Thread::finished, [&]{ timer.moveToThread(thread.thread()); });
      QObject::connect(&thread, &Thread::started, [&]{ dump(setPeriod); });
      QObject::connect(&timer, &QTimer::timeout, [&]{ dump(); });
   }
   thread.start();
   return app.exec();
}
