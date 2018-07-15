// https://github.com/KubaO/stackoverflown/tree/master/questions/html-get-24965972
#include <QtNetwork>
#include <functional>

void htmlGet(const QUrl &url, const std::function<void(const QString&)> &fun) {
   QScopedPointer<QNetworkAccessManager> manager(new QNetworkAccessManager);
   QNetworkReply *response = manager->get(QNetworkRequest(QUrl(url)));
   QObject::connect(response, &QNetworkReply::finished, [response, fun]{
      response->deleteLater();
      response->manager()->deleteLater();
      if (response->error() != QNetworkReply::NoError) return;
      auto const contentType =
            response->header(QNetworkRequest::ContentTypeHeader).toString();
      static QRegularExpression re("charset=([!-~]+)");
      auto const match = re.match(contentType);
      if (!match.hasMatch() || 0 != match.captured(1).compare("utf-8", Qt::CaseInsensitive)) {
         qWarning() << "Content charsets other than utf-8 are not implemented yet:" << contentType;
         return;
      }
      auto const html = QString::fromUtf8(response->readAll());
      fun(html); // do something with the data
   }) && manager.take();
}

int main(int argc, char *argv[])
{
   QCoreApplication app(argc, argv);
   htmlGet({"http://www.google.com"}, [](const QString &body){ qDebug() << body; qApp->quit(); });
   return app.exec();
}
