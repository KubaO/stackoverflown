// https://github.com/KubaO/stackoverflown/tree/master/questions/appmonitor-37524491
#include <QtWidgets>
#include <cstdlib>
#if defined(Q_OS_WIN32)
#include <windows.h>
#else
static void DebugBreak() { abort(); }
#endif

static int businessLogicMain(int &argc, char **argv) {
   QApplication app{argc, argv};
   qDebug() << __FUNCTION__ << app.arguments();
   QWidget w;
   QHBoxLayout layout{&w};
   QPushButton crash{"Crash"};  // purposefully crash for testing
   QPushButton quit{"Quit"};    // graceful exit, which doesn't need restart
   layout.addWidget(&crash);
   layout.addWidget(&quit);
   w.show();

   QObject::connect(&crash, &QPushButton::clicked, DebugBreak);
   QObject::connect(&quit, &QPushButton::clicked, &QCoreApplication::quit);
   return app.exec();
}

static char const kRunLogic[] = "run__business__logic";
static char const kRunLogicValue[] = "run__business__logic";

#if defined(Q_OS_WIN32)
static QString getWindowsCommandLineArguments() {
   const wchar_t *args = GetCommandLine();
   bool oddBackslash = false, quoted = false, whitespace = false;
   // skip the executable name according to Windows command line parsing rules
   while (auto c = *args) {
      if (c == L'\\')
         oddBackslash ^= 1;
      else if (c == L'"')
         quoted ^= !oddBackslash;
      else if (c == L' ' || c == L'\t')
         whitespace = !quoted;
      else if (whitespace)
         break;
      else
         oddBackslash = false;
      args++;
   }
   return QString::fromRawData(reinterpret_cast<const QChar*>(args), lstrlen(args));
}
#endif

static int monitorMain(int &argc, char **argv) {
#if !defined(Q_OS_WIN32)
   QStringList args;
   args.reserve(argc-1);
   for (int i = 1; i < argc; ++i)
     args << QString::fromLocal8Bit(argv[i]);
#endif
   QCoreApplication app{argc, argv};
   QProcess proc;
   auto onFinished = [&](int retcode, QProcess::ExitStatus status) {
      qDebug() << status;
      if (status == QProcess::CrashExit)
         proc.start();      // restart the app if the app crashed
      else
         app.exit(retcode); // no restart required
   };
   QObject::connect(&proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), onFinished);

   auto env = QProcessEnvironment::systemEnvironment();
   env.insert(kRunLogic, kRunLogicValue);
   proc.setProgram(app.applicationFilePath()); // logic and monitor are the same executable
#if defined(Q_OS_WIN32)
   SetErrorMode(SEM_NOGPFAULTERRORBOX);        // disable Windows error reporting
   proc.setNativeArguments(getWindowsCommandLineArguments()); // pass command line arguments natively
   env.insert("QT_LOGGING_TO_CONSOLE", "1");   // ensure that the debug output gets passed along
#else
   proc.setArguments(args);
#endif
   proc.setProcessEnvironment(env);
   proc.setProcessChannelMode(QProcess::ForwardedChannels);
   proc.start();
   return app.exec();
}

int main(int argc, char **argv) {
   if (qgetenv(kRunLogic) != kRunLogicValue)
      return monitorMain(argc, argv);
   else
      return qunsetenv(kRunLogic), businessLogicMain(argc, argv);
}
