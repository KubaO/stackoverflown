// https://github.com/KubaO/stackoverflown/tree/master/questions/metamethod-lookup-24577095
#include <QtCore>

class MyObject : public QObject {
   Q_OBJECT
public:
   Q_SLOT void aSlot() {}
   Q_SLOT void aSlot2(int) {}
   Q_SLOT int aSlot3(int) { return 0; }
   Q_SIGNAL void aSignal();
   Q_SIGNAL void aSignal2(int);
};

template <typename Func> int indexOfMethod(Func method)
{
   using FuncType = QtPrivate::FunctionPointer<Func>;
   int methodIndex = -1;
   void *metaArgs[] = {&methodIndex, reinterpret_cast<void **>(&method)};
   auto mo = FuncType::Object::staticMetaObject;
   mo.static_metacall(QMetaObject::IndexOfMethod, 0, metaArgs);
   return methodIndex;
}

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);
   qDebug() << indexOfMethod(&MyObject::aSlot)
            << indexOfMethod(&MyObject::aSlot3) << indexOfMethod(&MyObject::aSignal2);
   return 0;
}
#include "main.moc"
