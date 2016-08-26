#include <QScopedPointer>
#include <QDebug>

class T {
   Q_DISABLE_COPY(T)
public:
   T() { qDebug() << "Constructed" << this; }
   ~T() { qDebug() << "Destructed" << this; }
   void act() { qDebug() << "Acting on" << this; }
};

void foo(QScopedPointer<T> & p)
{
   using std::swap;
   QScopedPointer<T> local;
   swap(local, p);
   local->act();
}

int main()
{
   QScopedPointer<T> p(new T);
   foo(p);
   qDebug() << "foo has returned";
   return 0;
}
