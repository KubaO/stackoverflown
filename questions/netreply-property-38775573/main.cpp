// https://github.com/KubaO/stackoverflown/tree/master/questions/netreply-property-38775573

#if 0
// w/o signal mapper
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
   QObject::connect(&worker, SIGNAL(got(QNetworkReply*)), &app, SLOT(quit()));
   worker.newRequest(
            QUrl{"http://stackoverflow.com/questions/38775573/best-way-to-use-qsignalmapper"},
            &model);
   return app.exec();
}
#include "main.moc"

#endif

#if 1
// w/signal mapper
#include <QtNetwork>
#include <QStringListModel> // needed for Qt 4
using DataModel = QStringListModel;

class Worker : public QObject {
   Q_OBJECT
   QNetworkAccessManager m_manager;
   QSignalMapper m_mapper;
   // QObject::connect is not clever enough to know that QNetworkReply* is-a QObject*
   Q_SLOT void map(QNetworkReply* reply) { m_mapper.map(reply); }
   Q_SLOT void onFeedRetrieved(QObject * dataModelObject) {
      auto dataModel = qobject_cast<DataModel*>(dataModelObject);
      auto reply = qobject_cast<QNetworkReply*>(m_mapper.mapping(dataModelObject));
      qDebug() << dataModel << reply;
      emit got(reply);
   }
public:
   Worker(QObject * parent = nullptr) : QObject{parent} {
      connect(&m_manager, SIGNAL(finished(QNetworkReply*)), SLOT(map(QNetworkReply*)));
      connect(&m_mapper, SIGNAL(mapped(QObject*)), SLOT(onFeedRetrieved(QObject*)));
   }
   void newRequest(const QUrl & url, DataModel * dataModel) {
      QNetworkRequest request{url};
      auto reply = m_manager.get(request);
      // Ensure a unique mapping
      Q_ASSERT(m_mapper.mapping(dataModel) == nullptr);
      m_mapper.setMapping(reply, dataModel);
   }
   Q_SIGNAL void got(QNetworkReply*);
};

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   DataModel model;
   Worker worker;
   QObject::connect(&worker, SIGNAL(got(QNetworkReply*)), &app, SLOT(quit()));
   worker.newRequest(
            QUrl{"http://stackoverflow.com/questions/38775573/best-way-to-use-qsignalmapper"},
            &model);
   return app.exec();
}
#include "main.moc"
#endif
