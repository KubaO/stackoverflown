// https://github.com/KubaO/stackoverflown/tree/master/questions/appmonitor-37524491
#include <QtWidgets>
#include <cstdlib>

static int businessLogicMain(int &argc, char **argv) {
  QApplication app{argc, argv};
  QWidget w;
  QHBoxLayout layout{&w};
  QPushButton crash{"Crash"};  // purposefully crash for testing
  QPushButton quit{"Quit"};    // graceful exit, which doesn't need restart
  layout.addWidget(&crash);
  layout.addWidget(&quit);
  w.show();

  QObject::connect(&crash, &QPushButton::clicked, []{ abort(); });
  QObject::connect(&quit, &QPushButton::clicked, &app, &QCoreApplication::quit);
  return app.exec();
}

static auto const kRunLogic = QStringLiteral("run__business__logic");
static auto const kRunLogicValue = kRunLogic;

#if defined(Q_OS_WIN32)
#include <windows.h>
QString getWindowsCommandLine() {
  return QString::fromWCharArray(GetCommandLine());
}
int	setenv(const char *name, const char *value, int) {
  return _putenv_s(name, value);
}
#endif

static int monitorMain(int &argc, char **argv) {
  QCoreApplication app{argc, argv};
  QProcess proc;
  auto onFinished = [&](int retcode, QProcess::ExitStatus status) {
    if (status == QProcess::CrashExit)
      proc.start();      // restart the app if the app crashed
    else
      app.exit(retcode); // no restart required
  };
  QObject::connect(&proc, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), onFinished);

  auto env = QProcessEnvironment::systemEnvironment();
  env.insert(kRunLogic, kRunLogicValue);
  proc.setProgram(app.applicationFilePath()); // logic and monitor are the same executable
#if defined(Q_OS_WIN32)
  proc.setNativeArguments(getWindowsCommandLine());
#else
  proc.setArguments(app.arguments().mid(1));
#endif
  proc.setProcessEnvironment(env);
  proc.setProcessChannelMode(QProcess::ForwardedChannels);
  proc.start();
  return app.exec();
}

static bool checkForRunLogic() {
  auto hasRunLogic = QProcessEnvironment::systemEnvironment().value(kRunLogic) == kRunLogicValue;
  if (hasRunLogic)
    setenv(kRunLogic.toLocal8Bit(), "", true);
  return hasRunLogic;
}

int main(int argc, char **argv) {
  if (!checkForRunLogic())
    return monitorMain(argc, argv);
  else
    return businessLogicMain(argc, argv);
}

