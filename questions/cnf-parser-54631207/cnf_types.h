// cnf_types.h
#pragma once
#include <stdint.h>

typedef struct {} Clause;

typedef struct {
  size_t clause_count;
  size_t variable_count;
  Clause *clauses[1];
} CNF;
