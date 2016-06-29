// https://github.com/KubaO/stackoverflown/tree/master/questions/signal-macros-38107329
#include <QtCore>
#include <array>
#include <algorithm>

#define CREATE_SIGNALS\
   Q_SIGNAL void signal1(const QString & = QString());\
   Q_SIGNAL void signal2(int = 0);

struct Foo : QObject {
   CREATE_SIGNALS
   Q_OBJECT
};

struct Bar : QObject {
   CREATE_SIGNALS
   Q_OBJECT
};

int main()
{
   std::array<int, 4> s;
   Foo foo;
   Bar bar;
   s.fill(0);
   QObject::connect(&foo, &Foo::signal1, [&]{++s[0];});
   QObject::connect(&foo, &Foo::signal2, [&]{++s[1];});
   QObject::connect(&bar, &Bar::signal1, [&]{++s[2];});
   QObject::connect(&bar, &Bar::signal2, [&]{++s[3];});
   emit foo.signal1();
   emit foo.signal2();
   emit bar.signal1();
   emit bar.signal2();
   Q_ASSERT(std::all_of(std::begin(s), std::end(s), [](int val) { return val == 1; }));
}

#include "main.moc"
