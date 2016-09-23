//### controller.h
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QtNetwork>

class Controller : public QObject {
   Q_OBJECT
   QNetworkAccessManager manager{this};
   QNetworkRequest request;
   Q_SLOT void onReply(QNetworkReply *);
public:
   explicit Controller(QObject * parent = nullptr);
   Q_SLOT void get();
   Q_SIGNAL void busy();
   Q_SIGNAL void error(const QString &);
   Q_SIGNAL void values(const QString & name, const QString & gender, const QString & region);
};

#endif // CONTROLLER_H
