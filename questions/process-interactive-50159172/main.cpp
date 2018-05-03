// https://github.com/KubaO/stackoverflown/tree/master/questions/process-interactive-50159172
#include <QtCore>
#include <cstdio>
int main(int argc, char *argv[]) {
   QCoreApplication app{argc, argv};
   QProcess process;
   static QTextStream cout{stdout};

   auto commands = {"help", "exec !2", "exec !0", "help", "exec !1", "exec !3", "quit"};

   auto on = [&process](auto method, auto functor) { QObject::connect(&process, method, functor); };
   on(&QProcess::started, []{ qDebug() << "*Started"; });
   on(&QProcess::readyReadStandardOutput,
      [&process, &commands, it = commands.begin(), buf = QByteArray(), newLine = true]() mutable {
      auto chunk = process.readAllStandardOutput();
      buf += chunk;
      cout << chunk << flush;
      if (buf.endsWith("$ ") && it != commands.end()) {
         auto *command = *it++;
         process.write(command);
         process.write("\n");
         cout << command << endl;
         newLine = true;
         buf.clear();
      }
   });
   on(&QProcess::errorOccurred, &QCoreApplication::quit);
   on(QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &QCoreApplication::quit);

   qputenv("QT_LOGGING_TO_CONSOLE", "1");
   process.setProcessChannelMode(QProcess::ForwardedErrorChannel);
   process.start("python", {"test.py"}, QIODevice::ReadWrite | QIODevice::Text);
   return app.exec();
}
