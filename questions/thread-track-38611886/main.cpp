// https://github.com/KubaO/stackoverflown/tree/master/questions/thread-track-38611886
#include <QtConcurrent>

template <typename T>
class MainResult {
   Q_DISABLE_COPY(MainResult)
   T * m_obj;
public:
   template<typename... Args>
   MainResult(Args&&... args) : m_obj{ new T(std::forward<Args>(args)...) } {}
   MainResult(T * obj) : m_obj{obj} {}
   T* operator->() const { return m_obj; }
   operator T*() const { return m_obj; }
   T* operator()() const { return m_obj; }
   ~MainResult() { m_obj->moveToThread(qApp->thread()); }
};

struct Foo : QObject { Foo(int) {} };

QFuture<Foo*> test1() {
   return QtConcurrent::run([=]()->Foo*{ // explicit return type
      MainResult<Foo> obj{1};
      obj->setObjectName("Hello");
      return obj; // return by value
   });
}

QFuture<Foo*> test2() {
   return QtConcurrent::run([=](){ // deduced return type
      MainResult<Foo> obj{1};
      obj->setObjectName("Hello");
      return obj(); // return by call
   });
}

MainResult<Foo> obj{ new Foo{1} };

int main() {}
