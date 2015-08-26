#include <iostream>

int main() {
   struct {
      struct S {
         int member;
         S() { std::cout << "constructed" << std::endl; }
         ~S() { std::cout << "destructed" << std::endl; }
      } s;
   } instance1, instance2;
   std::cout << "body" << std::endl;
   return 0;
}
