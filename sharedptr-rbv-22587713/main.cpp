#include <QSharedPointer>
#include <memory>

QSharedPointer<int> answer1() {
  // shared-pointer-to-integer is on the heap
  std::shared_ptr<QSharedPointer<int>> ptr(new QSharedPointer<int>(new int));
  **ptr = 42;
  return *ptr;
}

QSharedPointer<int> answer2() {
  // shared-pointer-to-integer is a local value
  QSharedPointer<int> ptr = answer1();
  return ptr;
}

int main()
{
   Q_ASSERT(*answer2() == 42);
   return 0;
}
