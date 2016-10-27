// https://github.com/KubaO/stackoverflown/tree/master/questions/vector-append-40274282
#include <array>
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <type_traits>
#include <vector>

struct A {
   static int copies, moves;
   A() {}
   A(A&&) { moves++; }
   A(const A&) { copies++; }
   A& operator=(const A&) { copies++; return *this; }
   static void reset() { A::copies = 0; A::moves = 0; }
};
int A::copies, A::moves;

template<typename T>
class rref_capture
{
   T* ptr;
public:
   rref_capture(T&& x) : ptr(&x) {}
   operator T&& () const { return std::move(*ptr); } // restitute rvalue ref
};

template <typename T>
void append(std::vector<T> & v,
            typename std::decay<std::initializer_list<rref_capture<T>>>::type u) {
   v.reserve(v.size() + u.size());
   for (auto && item : u)
      v.push_back(std::move(item));
}

template <typename T, typename U>
void append(std::vector<T> & v, U && u) {
   v.reserve(v.size() + std::distance(std::begin(u), std::end(u)));
   for (auto & item : u)
      v.push_back(std::move(item));
}

template <typename T, typename U>
void append(std::vector<T> & v, U & u) {
   v.reserve(v.size() + std::distance(std::begin(u), std::end(u)));
   for (auto & item : u)
      v.push_back(item);
}

int main() {
   std::vector<A> vec;
   vec.reserve(100);

   A::reset();
   append(vec, {A(), A()});
   assert(A::copies == 0 && A::moves == 2 && vec.size() == 2);

   auto vec2 = vec;
   A::reset();
   append(vec, vec2);
   assert(A::copies == 2 && A::moves == 0 && vec.size() == 4);

   A::reset();
   append(vec, std::move(vec2));
   assert(A::copies == 0 && A::moves == 2 && vec.size() == 6);

   A::reset();
   append(vec, std::array<A,2>{A(), A()});
   assert(A::copies == 0 && A::moves == 2 && vec.size() == 8);

   const std::vector<A> cvec{2};
   A::reset();
   append(vec, cvec);
   assert(A::copies == 2 && A::moves == 0 && vec.size() == 10);

   A arr[2];
   A::reset();
   append(vec, arr);
   assert(A::copies == 2 && A::moves == 0 && vec.size() == 12);
}
