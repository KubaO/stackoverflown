// https://github.com/KubaO/stackoverflown/tree/master/questions/ns-meta-35505644
#include <QtCore>

int A_a, B_a;

class A : public QObject {
   Q_OBJECT
public:
   Q_INVOKABLE A(int a, QObject * parent = 0) : QObject{parent} {
      A_a = a;
   }
};

namespace NS {
class B : public A {
   Q_OBJECT
public:
   Q_INVOKABLE B(int a, QObject * parent = 0) : A{a, parent} {
      B_a = a;
   }
};
}

int main() {
   Q_ASSERT(A_a == 0);
   Q_ASSERT(B_a == 0);
   QScopedPointer<QObject> a {A::staticMetaObject.newInstance(Q_ARG(int, 10))};
   Q_ASSERT(A_a == 10);
   QScopedPointer<QObject> b {NS::B::staticMetaObject.newInstance(Q_ARG(int, 20))};
   Q_ASSERT(A_a == 20);
   Q_ASSERT(B_a == 20);
   QScopedPointer<QObject> c {b->metaObject()->newInstance(Q_ARG(int, 30))};
   Q_ASSERT(A_a == 30);
   Q_ASSERT(B_a == 30);
}

#include "main.moc"
