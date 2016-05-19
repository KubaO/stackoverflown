// https://github.com/KubaO/stackoverflown/tree/master/questions/thread-sigslot-37325348
#include <QtCore>

class Worker1 : public QObject {
   Q_OBJECT
public:
   Q_SIGNAL void sig1();
   Q_SLOT void slot1() {
      qDebug() << "slot in" << thread();
   }
};

class Worker2 : public QObject {
   Q_OBJECT
public:
   Worker2() {
      QTimer::singleShot(100, this, [this]{
         qDebug() << "emit sig2 in" << thread();
         emit sig2();
      });
   }
   Q_SIGNAL void sig2();
};

class Object : public QObject {
   Q_OBJECT
   Worker1 w1;
   Worker2 w2;
   QThread t1, t2;
   Q_SIGNAL void sig();
public:
   Object() {
      t1.setObjectName("t1");
      t2.setObjectName("t2");
      connect(&w2, &Worker2::sig2, &w1, &Worker1::slot1);
      connect(this, &Object::sig, &w1, &Worker1::slot1);
      w1.moveToThread(&t1);
      w2.moveToThread(&t2);
      t1.start();
      t2.start();
      QTimer::singleShot(1000, this, [this]{
         qDebug() << "emit sig in" << thread();
         emit sig();
      });
   }
   ~Object() { t1.quit(); t2.quit(); t1.wait(); t2.wait(); }
};

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   app.thread()->setObjectName("main_thread");
   Object obj;
   QTimer::singleShot(2000, &app, [&]{ app.quit(); });
   return app.exec();
}

#include "main.moc"
