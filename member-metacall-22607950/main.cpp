#include <QCoreApplication>
#include <QDebug>
#include <QMetaObject>

class Class : public QObject {
   Q_OBJECT
public:
   typedef void (Class::*Method)(const QString &);
private:
   void method(const QString & text) { qDebug() << text; }
   Q_SLOT void callSlot(const QString & text, Class::Method method) {
      (this->*method)(text);
   }
   Q_SIGNAL void callSignal(const QString & text, Class::Method method);
public:
   Class() {
      connect(this, SIGNAL(callSignal(QString,Class::Method)),
              SLOT(callSlot(QString,Class::Method)),
              Qt::QueuedConnection);
      emit callSignal("Hello", &Class::method);
   }
};
Q_DECLARE_METATYPE(Class::Method)

int main(int argc, char *argv[])
{
   qRegisterMetaType<Class::Method>();
   QCoreApplication a(argc, argv);
   Class c;
   QMetaObject::invokeMethod(&a, "quit", Qt::QueuedConnection);
   return a.exec();
}

#include "main.moc"
