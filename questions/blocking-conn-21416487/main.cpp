#include <QDebug>
#include <QThread>
#include <QCoreApplication>

class Master : public QObject {
  Q_OBJECT
public:
  Q_SIGNAL bool mySignal();
  Q_SLOT void process() {
    if (mySignal()) {
      qDebug() << "success!"; // this can be a blocking call
    }
    thread()->quit();
  }
};

class Slave : public QObject {
  Q_OBJECT
public:
  Q_SLOT bool mySlot() {
    return true;
  }
};

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  QThread masterThread, slaveThread;
  Master master;
  Slave slave;
  master.moveToThread(&masterThread);
  slave.moveToThread(&slaveThread);
  slave.connect(&master, SIGNAL(mySignal()), SLOT(mySlot()),
                Qt::BlockingQueuedConnection);
  masterThread.start();
  slaveThread.start();
  QMetaObject::invokeMethod(&master, "process");
  masterThread.wait();
  slaveThread.quit();
  slaveThread.wait();
  return 0;
}

#include "main.moc"
