//### controller.cpp
#include "controller.h"

Controller::Controller(QObject *parent) : QObject(parent)
{
   QUrlQuery query;
   query.addQueryItem("amount", "1");
   query.addQueryItem("region", "United States");
   QUrl url("http://uinames.com/api/");
   url.setQuery(query);
   request = QNetworkRequest(url);
   connect(&manager, &QNetworkAccessManager::finished, this, &Controller::onReply);
}

void Controller::onReply(QNetworkReply * reply) {
   if (reply->error() != QNetworkReply::NoError) {
      emit error(reply->errorString());
      manager.clearAccessCache();
   } else {
      //parse the reply JSON and display result in the UI
      auto jsonObject = QJsonDocument::fromJson(reply->readAll()).object();
      auto fullName = jsonObject["name"].toString();
      fullName.append(" ");
      fullName.append(jsonObject["surname"].toString());
      emit values(fullName, jsonObject["gender"].toString(), jsonObject["region"].toString());
   }
   reply->deleteLater();
}

void Controller::get() {
   manager.get(request);
}
