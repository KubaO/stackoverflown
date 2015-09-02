#include <QCoreApplication>
#include <QTimer>
#include <QThread>
#include <QDebug>

class Thread : public QThread {
   using QThread::run;
public:
   ~Thread() { quit(); wait(); }
};

class Robot : public QObject
{
   Q_OBJECT
   QTimer mTimer;
   int mCounter;
public:
   Robot(QObject *parent = 0) : QObject(parent), mTimer(this), mCounter(0) {
      connect(&mTimer, &QTimer::timeout, this, &Robot::update);;
      mTimer.start(1000);
   }
   Q_SLOT void update() {
      qDebug() << "updating";
      if (++mCounter > 5) qApp->exit();
   }
};

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);
   Robot robot;
   Thread thread;
   robot.moveToThread(&thread);
   thread.start();
   return a.exec();
}

#include "main.moc"
