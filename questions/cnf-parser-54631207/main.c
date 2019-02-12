// main.c
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Clause

typedef int ClauseLiteral;
static const int ClauseLiteralMax = INT_MAX;
static const int ClauseLiteralMin = INT_MIN;

typedef struct {
   size_t size;
   size_t capacity;  // does not include the terminating zero
   ClauseLiteral literals[1];
} Clause;

// Maximum capacity that doesn't overflow SIZE_MAX
static inline size_t Clause_max_capacity(void) {
   return (SIZE_MAX - sizeof(Clause)) / sizeof(ClauseLiteral);
}

static size_t Clause_size_for_(size_t const count_of_literals) {
   assert(count_of_literals);
   if (count_of_literals > Clause_max_capacity()) return 0;
   return sizeof(Clause) + count_of_literals * sizeof(ClauseLiteral);
}

static size_t Clause_next_capacity_(size_t const capacity) {
   assert(capacity);
   const size_t growth_factor = 2;
   if (capacity > Clause_max_capacity() / growth_factor) {
      if (capacity < Clause_max_capacity()) return Clause_max_capacity();
      return 0;
   }
   return capacity * growth_factor;
}

static Clause *new_Clause_impl_(size_t const capacity) {
   size_t const alloc_size = Clause_size_for_(capacity);
   assert(alloc_size);
   Clause *const clause = calloc(alloc_size, 1);  // is zero-terminated
   if (!clause) return NULL;
   clause->size = 0;
   clause->capacity = capacity;
   return clause;
}

Clause *new_Clause(void) { return new_Clause_impl_(4); }

void free_Clause(Clause *clause) { free(clause); }

/** Assures that the clause exists and has room for at least by items */
bool Clause_grow(Clause **const clause_ptr, size_t by) {
   assert(clause_ptr);
   if (!*clause_ptr) return (*clause_ptr = new_Clause_impl_(by));
   Clause *const clause = *clause_ptr;
   assert(clause->size <= clause->capacity);
   if (clause->size > (SIZE_MAX - by)) return false;               // overflow
   if (by > Clause_max_capacity()) return false;                   // won't fit
   if (clause->size > (Clause_max_capacity() - by)) return false;  // won't fit
   size_t const new_size = clause->size + by;
   assert(new_size <= Clause_max_capacity());
   if (new_size > clause->capacity) {
      size_t new_capacity = clause->capacity;
      while (new_capacity && new_capacity < new_size)
         new_capacity = Clause_next_capacity_(new_capacity);
      if (!new_capacity) return false;
      Clause *const new_clause = realloc(clause, Clause_size_for_(new_capacity));
      if (!new_clause) return false;
      *clause_ptr = new_clause;
   }
   (*clause_ptr)->literals[new_size] = 0;  // zero-terminate
   return true;
}

bool Clause_push_back(Clause **clause_ptr, ClauseLiteral literal) {
   assert(clause_ptr);
   assert(literal);  // zero-valued literals are not allowed within a clause
   if (!Clause_grow(clause_ptr, 1)) return false;
   (*clause_ptr)->literals[(*clause_ptr)->size++] = literal;
   return true;
}

// CNF

typedef struct {
   size_t clause_count;
   size_t variable_count;
   Clause *clauses[1];
} CNF;

static inline size_t CNF_max_clause_count() {
   return (SIZE_MAX - sizeof(CNF)) / sizeof(Clause *);
}

static size_t CNF_size_for_(size_t const clause_count) {
   if (clause_count >= CNF_max_clause_count()) return 0;
   return sizeof(CNF) + clause_count * sizeof(Clause *);
}

CNF *new_CNF(size_t variable_count, size_t clause_count) {
   if (variable_count > ClauseLiteralMax || -variable_count < ClauseLiteralMin)
      return NULL;
   size_t const size = CNF_size_for_(clause_count);
   if (!size) return NULL;
   CNF *const cnf = calloc(size, 1);
   if (!cnf) return NULL;
   cnf->variable_count = variable_count;
   cnf->clause_count = clause_count;
   return cnf;
}

void free_CNF(CNF *cnf) {
   if (!cnf) return;
   for (size_t i = 0; i < cnf->clause_count; ++i) free_Clause(cnf->clauses[i]);
   free(cnf);
}

// Parser

bool error(const char *msg) {
   fprintf(stderr, "Syntax error: %s\n", msg);
   return false;
}
void warning(const char *msg) { fprintf(stderr, "Syntax warning: %s\n", msg); }

struct Parser;

typedef void (*NextTerminal)(struct Parser *, void *);

typedef struct Parser {
   CNF *cnf;
   size_t clause_index;
   int term;
   int value;
   NextTerminal next_terminal;
   void *next_terminal_data;
   bool has_error;
} Parser;

typedef enum {
   p_COMMENT = 1,
   p_P = 2,
   p_IDENTIFIER = 4,
   p_INTEGER = 8,
   p_EOL = 16,
   p_EOF = 32,
   p_INVALID_INT = 64,
   p_JUNK = 128
} Terminal;

void init_Parser(Parser *parser, NextTerminal next_terminal, void *next_terminal_data) {
   memset(parser, 0, sizeof(*parser));
   parser->next_terminal = next_terminal;
   parser->next_terminal_data = next_terminal_data;
}

void next_terminal(Parser *parser) {
   parser->next_terminal(parser, parser->next_terminal_data);
}

bool accept(Parser *parser, int terms) {
   if (!(parser->term & terms)) return false;
   next_terminal(parser);
   return true;
}

bool accept_all(Parser *parser, int terms) {
   if (!accept(parser, terms)) return false;
   while (accept(parser, terms))
      ;
   return true;
}

bool expect(Parser *parser, int terms) {
   if (accept(parser, terms)) return true;
   error("Unexpected symbol.");
   return false;
}

int int_value(Parser *parser) { return parser->value; }

bool p_comments(Parser *parser) {
   while (accept(parser, p_COMMENT) && expect(parser, p_EOL | p_EOF))
      ;
   return true;
}

bool p_problem(Parser *parser) {
   int variable_count, clause_count;
   accept_all(parser, p_EOL);
   expect(parser, p_P);
   if (expect(parser, p_INTEGER))
      if ((variable_count = int_value(parser)) < 0)
         return error("Invalid variable count.");
   if (expect(parser, p_INTEGER))
      if ((clause_count = int_value(parser)) < 0) return error("Invalid clause count.");
   if (expect(parser, p_EOL | p_EOF)) parser->cnf = new_CNF(variable_count, clause_count);
   return parser->cnf;
}

bool p_clauses(Parser *parser) {
   int variable_count = parser->cnf->variable_count;
   int clause_count = parser->cnf->clause_count;
   while (clause_count) {
      accept_all(parser, p_EOL);
      if (!expect(parser, p_INTEGER)) return false;
      int variable = int_value(parser);
      if (variable < -parser->cnf->variable_count ||
          variable > parser->cnf->variable_count)
         return error("Variable out of range");
      if (variable != 0) {
         if (!Clause_push_back(&parser->cnf->clauses[parser->clause_index], variable))
            return error("Out of memory while reading clause.");
      } else {
         if (parser->cnf->clauses[parser->clause_index])
            parser->clause_index++;
         else
            warning("Skipped empty clause");
         clause_count--;
      }
   }
   if (clause_count) return error("Too few clauses present in the file.");
   return true;
}

bool p_cnf(Parser *parser) {
   next_terminal(parser);
   p_comments(parser);
   p_problem(parser);
   p_clauses(parser);
   accept_all(parser, p_EOL);
   expect(parser, p_EOF);
   return !parser->has_error;
}

typedef int (*Getc)(void *);

typedef struct Buffer {
   const char *buffer, *ptr, *end;
} Buffer;

int Buffer_getc(Buffer *buf) {
   if (buf->ptr >= buf->end) return EOF;
   return *buf->ptr++;
}

typedef struct Lexer {
   Getc getc;
   union {
      Buffer buffer;
      FILE *file;
   } source;
   bool line_start;
   bool detect_EOL;
   int c[2];
} Lexer;

bool isspace_not_EOL(int c) { return isspace(c) && c != '\n'; }
bool isident_first(int c) { return isalpha(c) || c == '_'; }
bool isident(int c) { return isalnum(c) || c == '_'; }

void init_Lexer(Lexer *lexer) {
   memset(lexer, 0, sizeof(*lexer));
   lexer->line_start = true;
}

int Lexer_file_getc(void *file) { return fgetc(file); }

void init_Lexer_file(Lexer *lexer, FILE *file) {
   init_Lexer(lexer);
   lexer->getc = Lexer_file_getc;
   lexer->source.file = file;
}

int Lexer_buffer_getc(void *buffer) { return Buffer_getc(buffer); }

void init_Lexer_buffer(Lexer *lexer, const char *buffer, size_t buffer_size) {
   init_Lexer(lexer);
   lexer->getc = Lexer_buffer_getc;
   lexer->source.buffer.buffer = buffer;
   lexer->source.buffer.ptr = buffer;
   lexer->source.buffer.end = buffer + buffer_size;
}

int L_getc(Lexer *lex) {
   int c = lex->c[0];
   if (c) {
      lex->c[0] = lex->c[1];
      lex->c[1] = 0;
      return c;
   }
   return lex->getc(&lex->source);
}

void L_ungetc(Lexer *lex, int c) {
   assert(!lex->c[1]);
   lex->c[1] = lex->c[0];
   lex->c[0] = c;
   return false;
}

int L_accept_p(Lexer *lex, bool (*predicate)(int)) {
   assert(predicate);
   int const c = L_getc(lex);
   if (predicate(c)) return c ? c : true;
   L_ungetc(lex, c);
   return 0;
}

int L_accept_all_p(Lexer *lex, bool (*predicate)(int)) {
   assert(predicate);
   do {
      int const c = L_getc(lex);
      if (!predicate(c)) {
         L_ungetc(lex, c);

      }
   }
   int r;
   while ((r = L_accept_p()))
}

void Lexer_next_terminal(Parser *parser, void *lex_) {
   Lexer *const lex = lex_;
   parser->term = 0;
   while (!parser->term) {
      int c = L_getc(lex);
      if (isspace_not_EOL(c))
         ;
      else if (c == '\n') {
         lex->line_start = true;
         if (lex->detect_EOL) parser->term = p_EOL;

      } else if (isdigit(c) || c == '-') {
         parser->term = p_INTEGER;
         bool negative = c == '-';
         unsigned int value = 0;
         if (negative) c = L_getc(lex);
         while (isdigit(c)) {
            unsigned int old_value = value;
            value = value * 10 + (c - '0');
            if (old_value && old_value >= value) parser->term = p_INVALID_INT;
            c = L_getc(lex);
         }
         if (!negative && value <= INT_MAX)
            parser->value = value;
         else if (negative && -value >= INT_MIN)
            parser->value = -value;
         else
            parser->term = p_INVALID_INT;
      } else if (c == 'c' && lex->line_start && L_accept_all_p(lex, is_not_end)) {
         parser->term = p_COMMENT;
         while ((c = L_getc(lex)) != EOF && c != '\n')
            ;
         L_ungetc(lex, c);
      } else if (c == 'p' && lex->line_start && L_acceptp(lex, isspace_not_EOL)) {
         parser->term = p_P;
         lex->past_line_start = true;
         c = L_getc(lex);
         if (!isspace_not_EOL(c)) {
            L_ungetc(lex, c);
            c = 'p';
            goto reparse;
         }
      }

      for (;;) {
         c = L_getc(lex);
         if (!isspace(c)) break;
         if (c == '\n' && (lex->line_start = true) && lex->detect_EOL) break;
      }
      if (c == EOF)
         parser->term = p_EOF;
      else if (c == '\n') {
         lex->line_start = true;
         if (!lex->detect_EOL) goto parse;
         if (lex) parser->term = p_EOL;
         lex->past_line_start = false;
      } else
         else if (isalpha(c) || c == '_') {
            parser->term = p_IDENTIFIER;
            lex->past_line_
         }
      else if (isdigit(c) || c == '-') {
         parser->term = p_INTEGER;
         lex->past_line_start = true;
         bool negative = c == '-';
         unsigned int value = 0;
         if (negative) c = getc(source);
         while (isdigit(c)) {
            unsigned int old_value = value;
            value = value * 10 + (c - '0');
            if (old_value && old_value >= value) parser->term = p_INVALID_INT;
            c = getc(source);
         }
         if (!negative && value <= INT_MAX)
            parser->value = value;
         else if (negative && -value >= INT_MIN)
            parser->value = -value;
         else
            parser->term = p_INVALID_INT;
      }
      else {
         parser->term = p_JUNK;
         while (!isspace_not_EOL(c)) c = getc(source);
         lex->c = c;
      }
   }
}

// Demo

const char file_contents[] =
    "c  simple_v3_c2.cnf\n"
    "c\n"
    "p cnf 3 2\n"
    "1 -3 0\n"
    "2 3 -1 0\n";

bool write_all(const char *file_name, const char *contents) {
   FILE *const file = fopen(file_name, "w");
   if (!file) return false;
   size_t const size = strlen(contents);
   size_t const written = fwrite(contents, 1, size, file);
   fclose(file);
   return written == size;
}

int main() {
#if 0
   const char *file_name = "simple_v3_c2.cnf";
   if (!write_all(file_name, file_contents)) abort();
   FILE *file = fopen(file_name, "r");
   if (!file) abort();
#endif

   Lexer lexer;
   Parser parser;
   init_Lexer_buffer(&lexer, &file_contents[0], sizeof(file_contents));
   init_Parser(&parser, Lexer_next_terminal, &lexer);
   p_cnf(&parser);

   return 0;
}
