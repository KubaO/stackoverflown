// https://github.com/KubaO/stackoverflown/tree/master/questions/json-serialize-44567345
#include <QtCore>
#include <cstdio>

struct User {
   QString name;
   int age;
   QJsonObject toJson() const {
      return {{"name", name}, {"age", age}};
   }
};

QJsonArray toJson(const QList<User> & list) {
   QJsonArray array;
   for (auto & user : list)
      array.append(user.toJson());
   return array;
}

int main() {
   QList<User> users{{"John Doe", 43}, {"Mary Doe", 44}};
   auto doc = QJsonDocument(toJson(users));
   std::printf("%s", doc.toJson().constData());
}
