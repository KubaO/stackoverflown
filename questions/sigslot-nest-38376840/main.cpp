// https://github.com/KubaO/stackoverflown/tree/master/questions/sigslot-nest-38376840
#include <QtCore>
struct Monitor {
   int & depth() { static int depth = 0; return depth; }
   const char * const msg;
   Monitor(const char * msg) : msg{msg} {
      qDebug().noquote().nospace() << QString(depth()++, ' ') << msg << " entered";
   }
   ~Monitor() {
      qDebug().noquote().nospace() << QString(--depth(), ' ') << msg << " left";
   }
};
struct Object : QObject {
   Q_SIGNAL void signal1();
   Q_SIGNAL void signal2();
   Q_SLOT void slot1() { Monitor mon{__FUNCTION__}; }
   Q_SLOT void slot2() { Monitor mon{__FUNCTION__}; }
   Q_SLOT void slot3() {
      Monitor mon{__FUNCTION__};
      emit signal2();
   }
   Q_OBJECT
};

int main() {
   Monitor mon{__FUNCTION__};
   Object obj;
   QObject::connect(&obj, &Object::signal1, &obj, &Object::slot1);
   QObject::connect(&obj, &Object::signal1, &obj, &Object::slot2);
   QObject::connect(&obj, &Object::signal1, &obj, &Object::slot3);
   QObject::connect(&obj, &Object::signal2, &obj, &Object::slot1);
   QObject::connect(&obj, &Object::signal2, &obj, &Object::slot2);
   emit obj.signal1();
}
#include "main.moc"
