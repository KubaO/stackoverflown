// token_mod.i
%module token_mod
%{#include "main.h"%}
%ignore Token;
%include "main.h"
%rename("%s") Token;

struct Token {
  %immutable word;
  %typemap (out, fragment="SWIG_FromCharPtrAndSize") const char *word {
    $result = SWIG_FromCharPtrAndSize($1, (arg1)->wordlen);
  }
  const char *word;
  %typemap (out) const char *word;
};

