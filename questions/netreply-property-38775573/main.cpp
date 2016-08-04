// https://github.com/KubaO/stackoverflown/tree/master/questions/netreply-property-38775573
#include <QtNetwork>
#include <QStringListModel> // needed for Qt 4

using DataModel = QStringListModel;
const char kDataModel[] = "dataModel";

class Worker : public QObject {
   Q_OBJECT
   QNetworkAccessManager m_manager;
   Q_SLOT void onFeedRetrieved(QNetworkReply* reply) {
      auto dataModelObject = qvariant_cast<QObject*>(reply->property(kDataModel));
      auto dataModel = qobject_cast<DataModel*>(dataModelObject);
      qDebug() << dataModel;
      emit got(reply);
   }
public:
   Worker(QObject * parent = nullptr) : QObject{parent} {
      connect(&m_manager, SIGNAL(finished(QNetworkReply*)),
              SLOT(onFeedRetrieved(QNetworkReply*)));
   }
   void newRequest(const QUrl & url, DataModel * dataModel) {
      QNetworkRequest request{url};
      auto reply = m_manager.get(request);
      reply->setProperty(kDataModel, QVariant::fromValue((QObject*)dataModel));
   }
   Q_SIGNAL void got(QNetworkReply*);
};

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   DataModel model;
   Worker worker;
   worker.newRequest(QUrl{"http://stackoverflow.com/questions/38775573/best-way-to-use-qsignalmapper"},
                     &model);
   QObject::connect(&worker, SIGNAL(got(QNetworkReply*)), &app, SLOT(quit()));
   return app.exec();
}

#include "main.moc"
