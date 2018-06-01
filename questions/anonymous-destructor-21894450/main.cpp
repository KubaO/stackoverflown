#define SECTION 70

#if SECTION==10
#include <iostream>
#include <cmath>
int main() {
   struct {
      struct S {
         double a;
         int b;
         S() : a(sqrt(4)), b(42) { std::cout << "constructed" << std::endl; }
         ~S() { std::cout << "destructed" << std::endl; }
      } s;
   } instance1, instance2;
   std::cout << "body" << std::endl;
}
#endif

#if SECTION==20
#include <iostream>
#include <cmath>
int main() {
   struct {
      double a { sqrt(4) };
      int b { []{
            std::cout << "constructed" << std::endl;
            return 42; }()
            };
   } instance1, instance2;
}
#endif

#if SECTION==30
#include <iostream>
#include <cmath>
int main() {
   struct {
      double a { sqrt(4) };
      int b { [this]{ constructor(); return 42; }() };
      void constructor() {
         std::cout << "constructed" << std::endl;
      }
   } instance1, instance2;
}
#endif

#if SECTION==40
#include <iostream>
#include <cmath>
struct Construct {
   template <typename T> Construct(T* instance) {
      instance->constructor();
   }
};

int main() {
   struct {
      double a { sqrt(4) };
      inc b { 42 };
      Construct c { this };
      void constructor() {
         std::cout << "constructed" << std::endl;
      }
   } instance1, instance2;
}
#endif

#if SECTION==50
#include <iostream>
#include <cmath>
struct ConstructDestruct {
   void * m_instance;
   void (*m_destructor)(void*);
   template <typename T> ConstructDestruct(T* instance) :
      m_instance(instance),
      m_destructor(+[](void* obj){ static_cast<T*>(obj)->destructor(); })
   {
      instance->constructor();
   }
   ~ConstructDestruct() {
      m_destructor(m_instance);
   }
};

int main() {
   struct {
      double a { sqrt(4) };
      int b { 42 };
      ConstructDestruct cd { this };

      void constructor() {
         std::cout << "constructed" << std::endl;
      }
      void destructor() {
         std::cout << "destructed" << std::endl;
      }
   } instance1, instance2;
   std::cout << "body" << std::endl;
}
#endif

#if SECTION==60
#include <iostream>
#include <cmath>
#include <cstddef>

template <std::ptrdiff_t> struct MInt {};

struct ConstructDestruct {
   void (*m_destructor)(ConstructDestruct*);
   template <typename T, std::ptrdiff_t offset> ConstructDestruct(T* instance, MInt<offset>) :
      m_destructor(+[](ConstructDestruct* self){
      reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(self) - offset)->destructor();
   })
   {
      instance->constructor();
   }
   ~ConstructDestruct() {
      m_destructor(this);
   }
};
#define offset_to(member)\
   (MInt<offsetof(std::remove_reference<decltype(*this)>::type, cd)>())

int main() {
   struct {
      double a { sqrt(4) };
      int b { 42 };
      ConstructDestruct cd { this, offset_to(cd) };
      void constructor() {
         a = 1;
         b = sqrt(4);
         std::cout << "constructed " << std::hex << (void*)this << std::endl;
      }
      void destructor() {
         std::cout << "destructed " << std::hex << (void*)this << std::endl;
      }
   } instance1, instance2;
   std::cout << "body" << std::endl;
}
#endif

#if SECTION == 70
#include <iostream>
#include <cmath>
#include <cstddef>

template <std::ptrdiff_t> struct MInt {};

struct ConstructDestruct {
   void (*m_destructor)(ConstructDestruct*);
   template <typename T, std::ptrdiff_t offset>
   ConstructDestruct(T* instance, MInt<offset>) :
      m_destructor(+[](ConstructDestruct* self){
         reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(self) - offset)->destructor();
      })
   {
      instance->constructor();
   }
   ~ConstructDestruct() {
      m_destructor(this);
   }
};
#define offset_to(member)\
   (MInt<offsetof(std::remove_reference<decltype(*this)>::type, member)>())

int main() {
   struct {
      double a { sqrt(4) };
      int b { 42 };
      ConstructDestruct cd { this, offset_to(cd) };
      void constructor() {
         std::cout << "constructed " << std::hex << (void*)this << std::endl;
      }
      void destructor() {
         std::cout << "destructed " << std::hex << (void*)this << std::endl;
      }
   } instance1, instance2;
   std::cout << "body" << std::endl;
}
#endif

//int offset { offsetof(std::remove_reference<decltype(*this)>::type, offset) };
