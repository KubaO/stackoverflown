// https://github.com/KubaO/stackoverflown/tree/master/questions/html-get-24965972
#include <QtNetwork>
#include <functional>

void htmlGet(const QUrl &url, const std::function<void(const QString&)> & fun) {
   QScopedPointer<QNetworkAccessManager> manager(new QNetworkAccessManager);
   QNetworkReply * response = manager->get(QNetworkRequest(QUrl(url)));
   QObject::connect(manager, &QNetworkAccessManager::finished, [manager, response]{
     response->deleteLater();
     manager->deleteLater();
     if (reponse->error() != QNetworkReply::NoError) return;
     QString contentType =
       response->header(QNetworkRequest::ContentTypeHeader).toString();
     if (!contentType.contains("charset=utf-8")) {
       qWarning() << "Content charsets other than utf-8 are not implemented yet.";
       return;
     }
     QString html = QString::fromUtf8(response->readAll());
     // do something with the data
   }) && manager.take();
}

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);



   return a.exec();
}
