// https://github.com/KubaO/stackoverflown/tree/master/questions/meta-derived-38268004
#include <QtCore>

void meta(QObject * obj) {
   auto mo = obj->metaObject();
   for (int i = 0; i < mo->propertyCount(); i++)
      qDebug() << mo->property(i).name() << mo->property(i).read(obj);
}

struct DataObject : QObject {
   Q_OBJECT
};

struct User : DataObject {
   Q_PROPERTY(int id MEMBER m_id)
   Q_OBJECT
   int m_id { 1 };
};

int main() {
   User user;
   Q_ASSERT(user.metaObject()->propertyCount() == 2);
   meta(&user);
}

#include "main.moc"
