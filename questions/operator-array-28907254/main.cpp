#include <iostream>

struct Class
{
   template<typename T>
   void operator&(T* const &p)
   {
      std::cout << "general pointer operator " << (*p) << std::endl;
   }

   template<typename T, unsigned int N>
   void operator&(T (&)[N])
   {
      std::cout << "general array operator " << N << std::endl;
   }
};

int main()
{
   int myarr[1] = { 2 };
   int* p = myarr;
   Class obj;

   obj & myarr;
   obj & p;

   return 0;
}
