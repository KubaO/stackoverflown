// https://github.com/KubaO/stackoverflown/tree/master/questions/imbue-constructor-35658459
#include <iostream>
#include <utility>
#include <type_traits>
using namespace std;

class Base {
   friend class MakeBase;
   void check() {
      cout << "check()" << endl;
      method();
   }
protected:
   Base() { cout << "Base()" << endl; }
   virtual void method() {}
public:
   virtual ~Base() {}
};

class MakeBase {
protected:
   static void check(Base * b) { b->check(); }
};

template <class C> class Make : public C, MakeBase {
public:
   template <typename... Args> Make(Args&&... args) : C(std::forward<Args>(args)...) {
      static_assert(std::is_base_of<Base, C>::value,
                    "Make requires a class derived from Base");
      check(this);
   }
};

class Derived : public Base {
   int a;
protected:
   Derived(int a) : a(a) { cout << "Derived() " << endl; }
   void method() override { cout << ">" << a << "<" << endl; }
};

int main()
{
   Make<Derived> d(3);
}

