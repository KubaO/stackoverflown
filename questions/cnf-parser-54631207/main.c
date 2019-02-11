// main.c
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int ClauseLiteral;
static const int ClauseLiteralMax = INT_MAX;

typedef struct {
  size_t size;
  size_t capacity; // does not include the terminating zero
  ClauseLiteral literals[1];
} Clause;

// Maximum capacity that doesn't overflow SIZE_MAX
static inline size_t Clause_max_capacity(void) {
  return (SIZE_MAX-sizeof(Clause))/sizeof(ClauseLiteral);
}

static size_t Clause_size_for_(size_t const count_of_literals) {
  assert(count_of_literals);
  if (count_of_literals > Clause_max_capacity()) return 0;
  return sizeof(Clause) + count_of_literals*sizeof(ClauseLiteral);
}

static size_t Clause_next_capacity_(size_t const capacity) {
  assert(capacity);
  const size_t growth_factor = 2;
  if (capacity > Clause_max_capacity()/growth_factor) {
    if (capacity < Clause_max_capacity()) return Clause_max_capacity();
    return 0;
  }
  return capacity * growth_factor;
}

static Clause *new_Clause_impl_(size_t const capacity) {
  size_t const alloc_size = Clause_size_for_(capacity);
  assert(alloc_size);
  Clause *const clause = calloc(alloc_size, 1); // is zero-terminated
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
  if (clause->size > (SIZE_MAX - by)) return false; // overflow
  if (by > Clause_max_capacity()) return false; // won't fit
  if (clause->size > (Clause_max_capacity() - by)) return false; // won't fit
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
  (*clause_ptr)->literals[new_size] = 0; // zero-terminate
  return true;
}

bool Clause_push_back(Clause **clause_ptr, ClauseLiteral literal) {
  assert(clause_ptr);
  assert(literal); // zero literals are not allowed within a clause
  if (!Clause_grow(clause_ptr, 1)) return false;
  (*clause_ptr)->literals[(*clause_ptr)->size++] = literal;
  return true;
}

typedef struct {
   size_t clause_count;
   size_t variable_count;
   Clause *clauses[1];
} CNF;

static inline size_t CNF_max_clause_count() {
  return (SIZE_MAX-sizeof(CNF))/sizeof(Clause*);
}

static size_t CNF_size_for_(size_t const clause_count) {
  if (clause_count >= CNF_max_clause_count()) return 0;
  return sizeof(CNF) + clause_count * sizeof(Clause*);
}

static CNF *new_CNF(size_t variable_count, size_t clause_count) {
  assert(variable_count <= ClauseLiteralMax);
  size_t const cnf_size = CNF_size_for_(clause_count);
  CNF *cnf = calloc(cnf_size, 1);
  if (!cnf) return NULL;
  cnf->variable_count = variable_count;
  cnf->clause_count = clause_count;
  return cnf;
}

typedef struct {
   CNF **cnf;
   size_t clause_index;
   bool parsing_literals;
} ParserState;

#include "cnf.c"
#include "cnf.h"

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
#ifndef NDEBUG
   ParseTrace(stderr, "Parse: ");
#endif
   const char *file_name = "simple_v3_c2.cnf";
   if (!write_all(file_name, file_contents)) abort();
   FILE *file = fopen(file_name, "r");
   if (!file) abort();

   ParserState state = {NULL, 0, false};
   void *parser = ParseAlloc(malloc);

   //Parse(parser, T_EOF, 0, &state);
   Parse(parser, C_LINE, 0, &state);
   Parse(parser, EOL, 0, &state);
   Parse(parser, P_TOKEN, 0, &state);
   Parse(parser, NATURAL, 0, &state);
   Parse(parser, 0, 0, &state);
   ParseFree(parser, free);
   return 0;
}
