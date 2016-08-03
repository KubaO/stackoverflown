// https://github.com/KubaO/stackoverflown/tree/master/questions/method-ptr-list-38718402
#include <cassert>
#include <vector>
#include <functional>
#include <type_traits>

struct Base {};
struct Derived : Base {
   int counter = 0;
   void method1() { counter += 10; }
   void method2() { counter += 100; }
};

template <typename B>
class HandlerList {
   using fun = std::function<void()>;
   std::vector<fun> m_handlers;
public:
   template <typename T,
             typename = typename std::enable_if<std::is_base_of<B, T>::value>::type>
   void add(T* instance, void(T::* member)()) {
      m_handlers.push_back(std::bind(member, instance));
   }
   template <typename T,
             typename = typename std::enable_if<std::is_base_of<B, T>::value>::type>
   void add(T* instance, std::initializer_list<void(T::*)()> members) {
      for (auto & member : members)
         add(instance, member);
   }
   void call() {
      for (auto const & handler : m_handlers)
         handler();
   }
};

int main()
{
   struct NotDerived { void foo() {} } nd;
   Derived d;
   HandlerList<Base> h;
   h.add(&d, &Derived::method1);
   h.add(&d, &Derived::method2);
   h.add(&d, { &Derived::method1, &Derived::method2 });
#if 0
   h.add(&nd, &NotDerived::foo); // this will fail to compile
#endif
   h.call();
   assert(d.counter == 220);
}
