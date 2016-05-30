// https://github.com/KubaO/stackoverflown/tree/master/questions/appmonitor-37524491
#include <QtWidgets>
#include <cstdlib>

auto const kRunSlave = QStringLiteral("--runSlave");

int slave() {
  QWidget w;
  QHBoxLayout layout{&w};
  QPushButton crash{"Crash"};
  QPushButton quit{"Quit"};
  layout.addWidget(&crash);
  layout.addWidget(&quit);
  w.show();
  QObject::connect(&crash, &QPushButton::clicked, []{ abort(); });
  QObject::connect(&quit, &QPushButton::clicked, qApp, &QCoreApplication::quit);
  return qApp->exec();
}

int master()
{
  QProcess proc;
  auto restart = [&](int, QProcess::ExitStatus status = QProcess::CrashExit) {
    if (status == QProcess::CrashExit)
      proc.start();
    else
      qApp->quit();
  };
  QObject::connect(&proc, (void(QProcess::*)(int,QProcess::ExitStatus))&QProcess::finished, restart);
  proc.setProgram(qApp->applicationFilePath());
  proc.setArguments(QStringList{kRunSlave});
  restart(0);
  return qApp->exec();
}

int main(int argc, char *argv[])
{
  QApplication app{argc, argv};
  if (app.arguments().length() > 1 && app.arguments().contains(kRunSlave))
    return slave();
  else
    return master();
}
