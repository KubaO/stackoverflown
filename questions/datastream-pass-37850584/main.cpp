// https://github.com/KubaO/stackoverflown/tree/master/questions/datastream-pass-37850584
#include <QtCore>
#include <cstdio>
#include <memory>

struct Class : public QObject {
   Q_SIGNAL void source(std::shared_ptr<QTextStream>);
   Q_SLOT void destination(std::shared_ptr<QTextStream> stream) {
      *stream << "Hello" << endl;
   }
   Q_OBJECT
};
Q_DECLARE_METATYPE(std::shared_ptr<QTextStream>)

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   Class c;
   c.connect(&c, &Class::source, &c, &Class::destination, Qt::QueuedConnection);
   auto out = std::make_shared<QTextStream>(stdout);
   emit c.source(out);
   QMetaObject::invokeMethod(&app, "quit", Qt::QueuedConnection);
   *out << "About to exec" << endl;
   return app.exec();
}
#include "main.moc"
