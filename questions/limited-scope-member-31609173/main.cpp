#include <utility>
#include <cassert>

template <typename T, typename Member, Member member>
class ScopedMember {
   T data;
public:
   explicit ScopedMember(const T & d) : data(d) {}
   explicit ScopedMember(T && d) : data(std::move(d)) {}
   ScopedMember() {}
   template <Member m, void(*)(char[member == m ? 1 : -1]) = (void(*)(char[1]))0>
   T & use() { return data; }
   template <Member m, void(*)(char[member == m ? 1 : -1]) = (void(*)(char[1]))0>
   const T & use() const { return data; }
};

class C {
public:
   C() : m_foo(-1) {}
   void granted() {
      auto & foo = m_foo.use<&C::granted>();
      foo = 5;
      assert(m_foo.use<&C::granted>() == 5);
   }
   void rejected() {
#if 0
      // Won't compile
      auto & foo = m_foo.use<&C::rejected>();
#endif
   }
private:
   ScopedMember<int, void(C::*)(), &C::granted> m_foo;
};

int main()
{
   C().granted();
   return 0;
}
