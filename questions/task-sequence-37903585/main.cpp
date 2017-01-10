// https://github.com/KubaO/stackoverflown/tree/master/questions/task-sequence-37903585
#include <QtCore>
#include <QtNetwork>
#include <type_traits>

template <typename T> struct SourceAction;
template<> struct SourceAction<QProcess> {
   using signal_type = void(QProcess::*)(int,QProcess::ExitStatus);
   static constexpr signal_type source(QProcess*) {
      return static_cast<signal_type>(&QProcess::finished); }
};
template<> struct SourceAction<QNetworkReply> {
   using signal_type = void(QNetworkReply::*)();
   static constexpr signal_type source(QNetworkReply*) { return &QNetworkReply::finished; }
};

template <typename T> struct TargetAction;
template<> struct TargetAction<QProcess> {
   struct slot_type {
      QProcess * process;
      void operator()() { process->start(); }
      slot_type(QProcess* process) : process(process) {}
   };
   static slot_type destination(QProcess * process) { return slot_type(process); }
};
template<> struct TargetAction<QCoreApplication> {
   using slot_type = void(*)();
   static constexpr slot_type destination(QCoreApplication*) { return &QCoreApplication::quit; }
};

// SFINAE
template <typename Src, typename Dst>
void proceed(Src * src, Dst * dst) {
   QObject::connect(src, SourceAction<Src>::source(src),
                    dst, TargetAction<Dst>::destination(dst));
}
template <typename Src, typename F>
void proceed(Src * src, F && f) {
   QObject::connect(src, SourceAction<Src>::source(src), std::forward<F>(f));
}

QNetworkReply * download(QNetworkAccessManager * mgr, const QUrl & url) {
   return mgr->get(QNetworkRequest{url});
}
QProcess * setup(QProcess * process, const QString & program, const QStringList & args) {
   process->setProgram(program);
   process->setArguments(args);
   return process;
}

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   if (app.arguments().count() > 1) return 0;

   QNetworkAccessManager mgr;
   QProcess process;

   proceed(download(&mgr, {"http://www.google.com"}), &process);
   proceed(setup(&process, app.applicationFilePath(), {"dummy"}), &app);
   proceed(&process, []{ qDebug() << "quitting"; });
   return app.exec();
}

//
// Initial code
//
#define main main1

struct Task : QObject {
   Q_SLOT void start() {}
   Q_SIGNAL void done();
   Q_OBJECT
};

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   using Q = QObject;
   Task task1, task2, task3;
   Q::connect(&task1, &Task::done, &task2, [&]{ task2.start(); });
   Q::connect(&task2, &Task::done, &task3, [&]{ task3.start(); });
   Q::connect(&task3, &Task::done, &app, [&]{ app.quit(); });
   return app.exec();
}

#undef main
#define main main2

template <typename F> void onDone(QProcess * process, QObject * dst, F && f) {
   using signal_type = void(QProcess::*)(int,QProcess::ExitStatus);
   QObject::connect(process, static_cast<signal_type>(&QProcess::finished),
                    dst, std::forward<F>(f));
}

template <typename F> void onDone(QNetworkReply * reply, QObject * dst, F && f) {
   QObject::connect(reply, &QNetworkReply::finished, dst, std::forward<F>(f));
}

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   QNetworkAccessManager mgr;
   auto download = mgr.get(QNetworkRequest{QUrl{"http://www.google.com"}});
   QProcess process;

   onDone(download, &process, [&]{ process.start(); });
   onDone(&process, &app, [&]{ app.quit(); });

   return app.exec();
}

#include "main.moc"
