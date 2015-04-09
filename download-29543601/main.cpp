#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTemporaryFile>
#include <QUrl>
#include <QByteArray>
#include <QTextStream>
#include <QDebug>
#include <cstdio>

QTextStream out(stdout);
QTextStream in(stdin);

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);
   QNetworkAccessManager mgr;

   auto url = QUrl("http://stackoverflow.com/questions/29543601/"
                   "executable-getting-somehow-corrupted-when-being-copied");
   auto reply = mgr.get(QNetworkRequest(url));

   QTemporaryFile file;
   if (!file.open()) {
      qDebug() << "Can't open file for writing.";
      return -1;
   }
   out << "Writing to: " << file.fileName() << endl;

   QObject::connect(reply, &QNetworkReply::downloadProgress, [](qint64 rx, qint64 total) {
      qDebug() << "Downloaded" << rx << "of" << total << "bytes";
   });

   QObject::connect(reply, &QIODevice::readyRead, [reply, &file]{
      auto data = reply->readAll();
      auto written = file.write(data);
      if (data.size() != written) {
         qDebug() << "Write failed, wrote" << written << "out of" << data.size() << "bytes.";
      } else {
         qDebug() << "Wrote " << written << "bytes.";
      }
   });

   QObject::connect(reply, &QNetworkReply::finished, [reply, &file]{
      if (reply->error() != QNetworkReply::NoError) {
         qDebug() << "The request was unsuccessful. Error:" << reply->error();
         qApp->quit();
         return;
      }
      if (file.flush()) {
         out << "Successfully wrote " << file.fileName();
         out << "\nPress Enter to remove the file and exit." << flush;
         in.readLine();
      } else {
         qDebug() << "The file flush has failed";
      }
      qApp->quit();
   });

   return a.exec();
}
