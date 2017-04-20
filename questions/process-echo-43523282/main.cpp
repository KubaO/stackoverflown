// https://github.com/KubaO/stackoverflown/tree/master/questions/process-echo-43523282
#include <QtCore>
#define dumpval(x) qDebug()<<#x<<'='<<x

void slave()
{
   QCoreApplication::setApplicationName("slave");
   qDebug()<<"started";
   QFile input, output;
   QDataStream inputStream{&input}, outputStream{&output};
   dumpval(input.open(stdin, QFile::ReadOnly));
   dumpval(output.open(stdout, QFile::WriteOnly));
   QByteArray data;
   do {
      inputStream >> data;
      outputStream << data;
      dumpval(data);
   } while (inputStream.status() == QDataStream::Ok && !data.isEmpty());
   dumpval(inputStream.status());
}

void master()
{
   QCoreApplication::setApplicationName("master");
   qDebug()<<"started";
   QProcess p;
   p.setProgram(QCoreApplication::applicationFilePath());
   p.setArguments({"slave"});
   p.setProcessChannelMode(QProcess::ForwardedErrorChannel);
   p.start();
   p.waitForStarted();

   QDataStream stream(&p);
   QByteArray data;
   stream << "this is a test" << QByteArray{};
   while (true) {
      stream.startTransaction();
      stream >> data;
      if (stream.commitTransaction()) {
         dumpval(data);
         if (data.isEmpty())
            break;
      } else
         p.waitForReadyRead();
   }
   p.waitForFinished();
}

int main(int argc, char** argv)
{
   QCoreApplication app(argc, argv);
   qSetMessagePattern("%{appname}: %{message}");
   if (app.arguments().size() < 2) master(); else slave();
   qDebug() << "stopped";
}
