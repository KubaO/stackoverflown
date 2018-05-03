// https://github.com/KubaO/stackoverflown/tree/master/questions/appmonitor-37524491
#include <QtWidgets>
#include <algorithm>
#include <cstdlib>

static auto const kRunLogic = QLatin1String("~~runBusinessLogic~~");

int businessLogicMain(int &argc, char **argv) {
  QApplication app{argc, argv};
  Q_ASSERT(argc == 1);
  Q_ASSERT(app.arguments().count() == 1); // the special argument was removed
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

int monitorMain(int &argc, char **argv) {
  QCoreApplication app{argc, argv};
  QProcess proc;
  auto onFinished = [&](int retcode, QProcess::ExitStatus status) {
    if (status == QProcess::CrashExit)
      proc.start();      // restart the app if the app crashed
    else
      app.exit(retcode); // no restart required
  };
  QObject::connect(&proc, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), onFinished);

  proc.setProgram(qApp->applicationFilePath()); // logic and monitor are the same executable
  proc.setArguments({kRunLogic});
  proc.setProcessChannelMode(QProcess::ForwardedChannels);
  proc.start();
  return app.exec();
}

bool checkForRunLogic(int &argc, char **argv) {
  static struct SwitchLocator {
    int &argc;
    char **const argv;
    char **end = {}, **arg = {};
    bool check() {
      end = argv+argc;
      arg = std::find(argv+1, end, kRunLogic);
      return arg != end;
    }
  } sw{argc, argv};
  if (sw.check()) {
    // The special switch is present - remove it after QCoreApplication constructor returns
    qAddPreRoutine([]{
      if (sw.check()) {
        // QApplication may have already removed the switch, depending on other argument processing
        std::move(sw.arg+1, sw.end, sw.arg);
        sw.argc--;
      }
    });
    return true;
  }
  return false;
}

int main(int argc, char **argv) {
  if (!checkForRunLogic(argc, argv))
    return monitorMain(argc, argv);
  else
    return businessLogicMain(argc, argv);
}
