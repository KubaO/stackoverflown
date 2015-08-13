#include <QList>
#include <QDebug>

class C {
public:
  static int ctr;
  C() { ctr ++; }
  ~C() { ctr --; qDebug() << (void*)this << "C instance destructed"; }
};
int C::ctr = 0;

int main() {
  auto c1 = new C;
  auto c2 = new C;
  auto list = QList<C*>() << c1 << c2;
  list.removeAll(c1);
  list.removeAll(c2);
  Q_ASSERT(list.isEmpty());
  Q_ASSERT(C::ctr == 2);
  // we'll happily leak both instances above

  auto list2 = QList<C*>() << new C << new C << new C;
  qDeleteAll(list2);
  Q_ASSERT(C::ctr == 2);
}
