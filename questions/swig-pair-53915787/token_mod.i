// https://github.com/KubaO/stackoverflown/tree/master/questions/swig-pair-53915787
%module token_mod

struct Token {
  %immutable word;
  %typemap (out, fragment="SWIG_FromCharPtrAndSize") const char *word {
    $result = SWIG_FromCharPtrAndSize($1, (arg1)->wordlen);
  }
  const char *word;
};

struct Token *get_token(void);

%{
struct Token {
  const char *word;
  unsigned short wordlen;
};

struct Token *get_token(void) {
  struct Token *r = malloc(sizeof(struct Token));
  r->word = "1234";
  r->wordlen = 2;
  return r;
}
%}
