// https://github.com/KubaO/stackoverflown/tree/master/questions/function-pointer-11-34387119
#include <functional>
#include <cstdio>

struct Calculate {
  template <typename F> int run(F && f) {
    return f(1, 2);
  }
};

int f1(int, int) { return 0; }

struct F2 {
  int operator()(int, int) { return 0; }
};

int main() {
  Calculate calc;
  // pass a C function pointer
  calc.run(f1);
  // pass a C++98 functor
  calc.run(F2());
  // pass a C++11 stateless lambda
  calc.run(+[](int a, int b) -> int { return a-b; });
  // pass a C++11 stateful lambda
  int k = 8;
  calc.run([k](int a, int b) -> int { return a*b+k; });
}

