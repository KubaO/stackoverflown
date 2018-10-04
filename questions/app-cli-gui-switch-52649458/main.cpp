// https://github.com/KubaO/stackoverflown/tree/master/questions/app-cli-gui-switch-52649458
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets>
#endif

struct Options {
   bool cli;
};

static Options parseOptionsQt4() {
   Options opts = {};
   for (auto arg : QCoreApplication::arguments().mid(1)) {
      if (arg == "--cli")
         opts.cli = true;
      else
         qFatal("Unknown option %s", arg.toLocal8Bit().constData());
   }
   return opts;
}

static Options parseOptions() {
   if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0)) return parseOptionsQt4();
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
   Options opts = {};
   QCommandLineParser parser;
   QCommandLineOption cliOption("cli", "Start in command line mode.");
   parser.addOption(cliOption);
   parser.process(*qApp);
   opts.cli = parser.isSet(cliOption);
   return opts;
#endif
}

int main(int argc, char *argv[]) {
   QScopedPointer<QCoreApplication> app(new QCoreApplication(argc, argv));
   auto options = parseOptions();
   if (options.cli) {
      qDebug() << "cli";
   } else {
      qDebug() << "gui";
      app.reset();
      app.reset(new QApplication(argc, argv));
   }

   if (qobject_cast<QApplication *>(qApp))
      QMessageBox::information(nullptr, "Hello", "Hello, World!");
   QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
   return app->exec();
}
