#include <QObject>
#include <QNetworkReply>
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QApplication>


class ConnectNet : public QObject
{
   Q_OBJECT
   QNetworkAccessManager m_manager;
public:
   ConnectNet(QObject * parent = 0) : QObject(parent) {
      QObject::connect(&m_manager, &QNetworkAccessManager::finished, [this](QNetworkReply * reply) {
         if (reply->error() == QNetworkReply::NoError)
            emit replyAvailable(QString::fromUtf8(reply->readAll()));
      });
   }
signals:
   void replyAvailable(const QString & reply);
public slots:
   void sendRequest(const QString url) {
      QNetworkRequest request;
      request.setUrl(QUrl(url));
      request.setRawHeader("User-Agent", "MyLittleAgent");
      m_manager.get(request);
   }
};



int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   ConnectNet connectNet;

   return a.exec();
}

#include "main.moc"
