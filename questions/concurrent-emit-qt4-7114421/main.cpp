#if 0

// https://github.com/KubaO/stackoverflown/tree/master/questions/concurrent-emit-qt4-7114421
#include <QtCore>

class Helper : public QObject {
   Q_OBJECT
public:
   int n = 0;
   Q_SLOT void increment() {
      Q_ASSERT(QThread::currentThread() == qApp->thread());
      n++;
   }
};

int main(int argc, char **argv)
{
   QCoreApplication app(argc, argv);
   Helper helper;
   Q_ASSERT(helper.n == 0);
   QtConcurrent::run([&]{
      Q_ASSERT(QThread::currentThread() != qApp->thread());
      QObject src;
      QObject::connect(&src, SIGNAL(destroyed(QObject*)), &helper, SLOT(increment()));
      QObject::connect(&src, SIGNAL(destroyed(QObject*)), &app, SLOT(quit()));
   });
   app.exec();
   Q_ASSERT(helper.n == 1);
}

#include "main.moc"

#else

#include <QtConcurrent>

int main(int argc, char **argv)
{
   QCoreApplication app(argc, argv);
   int n = 0;
   Q_ASSERT(n == 0);
   QtConcurrent::run([&]{
      Q_ASSERT(QThread::currentThread() != qApp->thread());
      QObject src;
      QObject::connect(&src, &QObject::destroyed, &app, [&]{
         Q_ASSERT(QThread::currentThread() == qApp->thread());
         n ++;
         qApp->quit();
      });
   });
   app.exec();
   Q_ASSERT(n == 1);
}

#endif
