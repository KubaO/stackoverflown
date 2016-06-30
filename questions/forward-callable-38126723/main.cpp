// https://github.com/KubaO/stackoverflown/tree/master/questions/forward-callable-38126723
#include <QtCore>

template <typename C>
void onTimeout(int msec, C && callable) {
   QTimer::singleShot(msec, std::move(callable));
}

template <typename C>
void onTimeout(int msec, QObject * context, C && callable) {
   QTimer::singleShot(msec, context, std::move(callable));
}

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   QObject context;
   QThread thread;
   context.moveToThread(&thread);
   thread.start();
   onTimeout(1000, []{ qDebug() << "T+1s"; });
   onTimeout(2000, &context, [&]{ qDebug() << "T+2s"; thread.quit(); });
   QObject::connect(&thread, &QThread::finished, &app, &QCoreApplication::quit);
   return app.exec();
}
