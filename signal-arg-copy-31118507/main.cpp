#include <QCoreApplication>
#include <QDebug>
#include <QThread>

class Copyable {
public:
   Copyable() {}
   Copyable(const Copyable & src) {
      qDebug() << static_cast<const void*>(&src) << "was copied to"
               << static_cast<void*>(this) << "in thread" << QThread::currentThread();
   }
};
Q_DECLARE_METATYPE(Copyable)

class Object : public QObject {
   Q_OBJECT
public:
   Q_SIGNAL void source(const Copyable &);
   Q_SLOT void sink(const Copyable & data) {
      qDebug() << "got" << static_cast<const void*>(&data) << "in thread"
               << QThread::currentThread();
      qApp->quit();
   }
};

class Thread : public QThread { public: ~Thread() { quit(); wait(); } };

int main(int argc, char *argv[])
{
   QCoreApplication app(argc, argv);
   Copyable data;
   qDebug() << "data is at" << static_cast<void*>(&data) << "in main thread" << app.thread();
   qRegisterMetaType<Copyable>();
   Object o1, o2;
   Thread thread;
   o2.moveToThread(&thread);
   thread.start();
   o2.connect(&o1, &Object::source, &o2, &Object::sink);
   emit o1.source(data);
   return app.exec();
}

#include "main.moc"
