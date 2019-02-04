// https://github.com/KubaO/stackoverflown/tree/master/questions/swig-pair-53915787
#include <assert.h>
#include <stdlib.h>
#include <Python.h>
#include "main.h"

struct Token *make_token(void) {
  struct Token *r = malloc(sizeof(struct Token));
  r->word = "1234";
  r->wordlen = 2;
  return r;
}

char *word_check;

#if PY_VERSION_HEX >= 0x03000000
#  define SWIG_init    PyInit__token_mod
PyObject*
#else
#  define SWIG_init    init_token_mod
void
#endif
SWIG_init(void);

int main()
{
   PyImport_AppendInittab("_token_mod", SWIG_init);
   Py_Initialize();
   PyRun_SimpleString(
            "import sys\n"
            "sys.path.append('.')\n"
            "import token_mod\n"
            "from token_mod import *\n"
            "token = make_token()\n"
            "cvar.word_check = token.word\n");
   assert(word_check && strcmp(word_check, "12") == 0);
   Py_Finalize();
   return 0;
}
