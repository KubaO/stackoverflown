// https://github.com/KubaO/stackoverflown/tree/master/questions/cstr-alloc-32379663
#include <string.h>
#include <stdlib.h>

/// Returns a newly allocated "foo". The user must free it.
char* func2(void) {
  return strdup("foo");
}

/// Returns a newly allocated "foobar". The user must free it.
char* func1(void) {
  char* str1 = func2();
  const char str2[] = "bar";
  char* str = malloc(strlen(str1) + sizeof(str2));
  strcat(strcpy(str, str1), str2);
  free(str1);
  return str;
}

int main() {
  char* str = func1();
  printf("%s\n", str);
  free(str);
}
