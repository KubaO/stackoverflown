#include <stdio.h>

const char a[] = "a";

static const char b[] = "b";

void test(const char * arg)
{
   const char c[] = "c1";
   printf("1-. a=%s b=%s c=%s arg=%s\n", a,b,c,arg);
   const char a[] = "a1";
   static const char b[] = "b1";
   // arg is present in this scope, we can't redeclare it
   printf("1+. a=%s b=%s c=%s arg=%s\n", a,b,c,arg);
   {
      const char a[] = "a2";
      const char b[] = "b2";
      const char arg[] = "arg2";
      const char c[] = "c2";
      printf("2-. a=%s b=%s c=%s arg=%s\n", a,b,c,arg);
      {
         static const char a[] = "a3";
         const char b[] = "b3";
         static char arg[] = "arg3";
         static const char c[] = "c3";
         printf("3. a=%s b=%s c=%s arg=%s\n", a,b,c,arg);
      }
      printf("2+. a=%s b=%s c=%s arg=%s\n", a,b,c,arg);
   }
   printf("1++. a=%s b=%s c=%s arg=%s\n", a,b,c,arg);
}

int main(void)
{
   test("arg");
   return 0;
}

