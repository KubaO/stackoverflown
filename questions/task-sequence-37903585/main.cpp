// https://github.com/KubaO/stackoverflown/tree/master/questions/task-sequence-37903585
#include <QtCore>
#include <QtNetwork>
#include <type_traits>

template <typename T> struct SourceAction;
template<> struct SourceAction<QProcess> {
   typedef void(QProcess::* signal_type)(int,QProcess::ExitStatus);
   static constexpr signal_type source(QProcess*) { return static_cast<signal_type>(&QProcess::finished); }
};
template<> struct SourceAction<QNetworkReply> {
   typedef void(QNetworkReply::* signal_type)();
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
   typedef void(*slot_type)();
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
   QObject::connect(src, SourceAction<Src>::source(src), std::move(f));
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


