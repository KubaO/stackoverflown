// https://github.com/KubaO/stackoverflown/tree/master/questions/swig-pair-53915787
#include <stdio.h>
#include <stdlib.h>
#include <Python.h>

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
            "token = get_token()\n"
            "assert token.word == '12'\n");
   Py_Finalize();
   return 0;
}
