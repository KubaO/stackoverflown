#include <QCoreApplication>
#include <QThreadPool>
#include <QRunnable>
#include <QBasicTimer>
#include <QDebug>
#include <cstdlib>

class Thread : public QThread {
public:
  using QThread::msleep;
};

class IpData {
  QString m_value;
public:
  IpData(const QString & str) : m_value(str) {}
  const QString & value() const { return m_value; }
};

class IpRunnable : public QRunnable, IpData {
  void run() {
    qDebug() << "[" << QThread::currentThreadId()
             << "] processing ip" << value();
    Thread::msleep(qrand() < (RAND_MAX/4.0) ? 0 : 100);
  }
public:
  IpRunnable(const IpData & data) : IpData(data) {}
  IpRunnable(const QString & str) : IpData(str) {}
};

class Test : public QObject {
  Q_OBJECT
  int i;
  QBasicTimer timer;
  void timerEvent(QTimerEvent * t) {
    if (t->timerId() != timer.timerId()) return;
    QThreadPool::globalInstance()->start(new IpRunnable(QString::number(i)));
    if (++i > 100) qApp->quit();
  }
public:
  Test() : i(0) { timer.start(20, this); }
};

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  QThreadPool::globalInstance()->setMaxThreadCount(5);
  Test test;
  return app.exec();
}

#include "main.moc"
