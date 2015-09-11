// https://github.com/KubaO/stackoverflown/tree/master/questions/32484688
#include <iostream>

template <typename T, int N> static char * W__(const char (&src)[N], T) {
   static char storage[N];
   strcpy(storage, src);
   return storage;
}

#define W(x) W__(x, []{})

char * argv[] = {
   W("foo"),
   W("bar")
};

int main()
{
   std::cout << argv[0] << " " << argv[1] << std::endl;
}

