// https://github.com/KubaO/stackoverflown/tree/master/questions/countdown-latch-38362044
#include <climits>
#include <QSemaphore>

class CountDownLatch {
  Q_DISABLE_COPY(CountDownLatch)
  QSemaphore m_sem{INT_MAX};
public:
  CountDownLatch() {}
  ~CountDownLatch() {
     m_sem.acquire(INT_MAX);
     m_sem.release(INT_MAX);
  }
  class Locker {
    CountDownLatch * sem;
  public:
    Locker(const Locker & other) : sem{other.sem} { sem->m_sem.acquire(); }
    Locker(Locker && other) : sem{other.sem} { other.sem = nullptr; }
    Locker(CountDownLatch * sem) : sem{sem} { sem->m_sem.acquire(); }
    ~Locker() { if (sem) sem->m_sem.release(); }
  };
  Locker lock() { return Locker{this}; }
};

#include <QtConcurrent>

struct MyClass {
  CountDownLatch m_latch;
  MyClass() {
    auto lock = m_latch.lock(); // must be taken here
    QtConcurrent::run([this, lock]{
      // DON'T lock here, you'll have a race!
      QThread::sleep(10);
    });
  }
};

int main() {
  MyClass a;
}
