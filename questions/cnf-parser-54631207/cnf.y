// cnf.y
%extra_argument {ParserState *s}
%token_type {int}

cnf ::= header clauses.
header ::= comments problem.
comments ::= comments comment. { printf(stderr, "comments!\n"); }
comments ::= .
comment ::= C_LINE EOL.
problem ::= P_TOKEN NATURAL(V) NATURAL(C) EOL. { *s->cnf = new_CNF(V, C); }
clauses ::= clauses clause.
clauses ::= clause.
clause ::= literals ZERO. { s->clause_index++; }
literals ::= literals NONZERO(L). {
   Clause_push_back(&(*s-> cnf)->clauses[s->clause_index++], L);
}
literals ::= NONZERO.

%parse_failure {
   fprintf(stderr, "Syntax error.\n");
   abort();
}

