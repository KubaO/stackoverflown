// https://github.com/KubaO/stackoverflown/tree/master/questions/variant-convert-38071414
#include <QtCore>

struct Foo {
   int a;
   Foo() = default;
   explicit Foo(int a) : a(a) {}
};
QDebug operator<<(QDebug debug, const Foo & f) {
   return debug << f.a;
}
Q_DECLARE_METATYPE(Foo)

struct Visitor
{
   Q_GADGET
   Q_INVOKABLE void visit(int i) { qDebug() << "got int" << i; }
   Q_INVOKABLE void visit(const QString & s) { qDebug() << "got string" << s; }
   Q_INVOKABLE void visit(const Foo & f) { qDebug() << "got foo" << f; }
};

template <typename V>
void visit(const QVariant & variant, const V & visitor) {
   auto & metaObject = V::staticMetaObject;
   for (int i = 0; i < metaObject.methodCount(); ++i) {
      auto method = metaObject.method(i);
      if (method.parameterCount() != 1)
         continue;
      auto arg0Type = method.parameterType(0);
      if (variant.type() != (QVariant::Type)arg0Type)
         continue;
      QGenericArgument arg0{variant.typeName(), variant.constData()};
      if (method.invokeOnGadget((void*)&visitor, arg0))
         break;
   }
}

int main() {
   visit(QVariant{1}, Visitor{});
   visit(QVariant{QStringLiteral("foo")}, Visitor{});
   visit(QVariant::fromValue(Foo{10}), Visitor{});
}

#include "main.moc"
