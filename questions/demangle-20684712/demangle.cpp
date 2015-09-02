#include <iostream>
#ifndef _MSC_VER
#include <cxxabi.h>
#endif

struct QEvent { virtual ~QEvent() {} };

struct MyEvent : public QEvent {};

#ifndef _MSC_VER
template <typename T> void dumpType(T val)
{
  int status;
  char * realname = abi::__cxa_demangle(typeid(val).name(), 0, 0, &status);
  std::cout << realname << std::endl;
  free(realname); //important!
}

template <typename T> void dumpType(T *ptr)
{
  int status;
  char * realname = abi::__cxa_demangle(typeid(*ptr).name(), 0, 0, &status);
  std::cout << realname << std::endl;
  free(realname); //important!
}
#else
template <typename T> void dumpType(T val)
{
  std::cout << typeid(val).name() << std::endl;
}
template <typename T> void dumpType(T *ptr)
{
  std::cout << typeid(*ptr).name() << std::endl;
}
#endif

int main() {
  QEvent * base = new QEvent;
  QEvent * der = new MyEvent;
  dumpType(int());
  dumpType(base);
  dumpType(der);
}

