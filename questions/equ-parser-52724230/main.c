#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node Node;
typedef struct List List;
typedef struct Parser Parser;

// List of AST Nodes

struct List {
   size_t size;
   size_t capacity;
   Node **data;
};

void List_grow_(List *list, size_t capacity) {
   assert(list);
   if (capacity >= list->capacity)
      list->data = realloc(list->data, capacity * sizeof(Node *));
}

void List_construct(List *list, size_t capacity) {
   memset(list, 0, sizeof(List));
   List_grow_(list, capacity);
}

void List_destruct(List *list) {
   if (!list) return;
   free(list->data);
}

List *new_List(size_t capacity) {
   List *list = malloc(sizeof(List));
   List_construct(list, capacity);
   return list;
}

void delete_List(List *list) {
   List_destruct(list);
   free(list);
}

Node **List_find(List *list, Node *item) {
   Node **const end = list->data + list->size;
   for (Node **p = list->data; p != end; p++)
      if (*p == item) return p;
   return NULL;
}

void List_push_back(List *list, Node *item) {
   assert(!List_find(list, item));
   if (list->size >= list->capacity) List_grow_(list, 2 * list->capacity);
   list->data[list->size++] = item;
}

bool List_remove(List *list, Node *item) {
   size_t i = list->size;
   while (i--)
      if (list->data[i] == item) {
         memmove(list->data + i, list->data + i + 1, list->size - i - 1);
         list->size--;
         return true;
      }
   return false;
}

size_t List_size(List *list) { return list->size; }

struct Node *List_at(List *list, size_t i) {
   assert(list && i < list->size);
   return list->data[i];
}

// AST Node

typedef enum { MonomialNode, SumNode, EquationNode, SystemNode } NodeType;

struct Node {
   Node *parent;
   List children;
   double factor;
   int exponent;
   NodeType type;
   char variable;
};

void Node_construct(Node *node, NodeType type, Node *parent) {
   memset(node, 0, sizeof(Node));
   node->type = type;
   node->parent = parent;
   List_construct(
       &node->children,
       (type == SumNode) ? 1 : (type == EquationNode) ? 2 : (type == SystemNode) ? 1 : 0);
}

void Node_destruct(Node *node) {
   if (!node) return;
   if (node->parent) List_remove(&node->parent->children, node);
   size_t size = List_size(&node->children);
   while (size) Node_destruct(List_at(&node->children, --size));
   List_destruct(&node->children);
}

Node *new_Node(NodeType type, Node *parent) {
   Node *node = malloc(sizeof(Node));
   Node_construct(node, type, parent);
   return node;
}

void delete_Node(Node *node) {
   Node_destruct(node);
   free(node);
}

// Parser

typedef intptr_t rule_t;
typedef const rule_t rule_ct;

#define RULES                                                                         \
   RULE(System), RULE(Equation), RULE(Sum), RULE(Term), RULE(Monomial), RULE(Equals), \
       RULE(Operator), RULE(Factor), RULE(Variable), RULE(Number), RULE(Exponent),    \
       RULE(Sign), RULE(Integer), RULE(DIGIT), RULE(PLUSMINUS), RULE(WS), RULE(LWS),  \
       RULE(NL), RULE(WSP), RULE(EOF), RULE(LF)
#define RULE(name) name##_[]
extern const char RULES;
#undef RULE
#define RULE(name) name##_[] = #name
const char RULES;
#undef RULE
#undef RULES

// ABNF syntax; negative characters: literal terminals; '^' precedes a literal terminal;
// clang-format off
rule_ct syntax[] = {
   System_,       '=', LWS_, '[', Equation_, '*', '(', NL_, Equation_, ')', ']', LWS_, EOF_, 0,
   Equation_,     '=', Sum_, Equals_, Sum_, 0,
   Sum_,          '=', Monomial_, '*', Term_, WS_, 0,
   Term_,         '=', Operator_, Monomial_, 0,
   Monomial_,     '=', Factor_, '[', Variable_, ']', '/', Variable_, 0,
   Equals_,       '=', WS_, -'=', 0,
   Operator_,     '=', WS_, PLUSMINUS_, 0,
   Factor_,       '=', WS_, '[', Sign_, ']', WS_, Number_, 0,
   Variable_,     '=', WS_, -'a', '-', -'z', 0,
   Number_,       '=', WS_, '(', Integer_, '[', -'.', Integer_, ']', '/', -'.', Integer_, ')', '[', Exponent_, ']', 0,
   Exponent_,     '=', -'e', '[', Sign_, ']', Integer_, ']', 0,
   Sign_,         '=', PLUSMINUS_, 0,
   Integer_,      '=', '1', '*', DIGIT_, 0,
   DIGIT_,        '=', -'0', '-', -'9', 0,
   PLUSMINUS_,    '=', -'+', '/', -'-', 0,
   WS_,           '=', '*', WSP_, 0,
   LWS_,          '=', '*', '(', WSP_, '/', LF_, ')', 0,
   NL_,           '=', WS_, LF_, WS_, 0,
   WSP_,          '=', -' ', '/', -'\t', 0,
   EOF_,          '=', '^', EOF, 0,
   LF_,           '=', -'\n', 0
};
// clang-format on

typedef enum { ParseDone, ParseOK, ParseFail } ParserResult;

struct Parser {
   Node system;
   Node *equation;
   Node *sum;
   Node *monomial;
   ParserResult (*state)(Parser *, int);
   ParserResult result;
   int substate;
   int ch;
   int integer;
};

#if 0
ParserResult parse_EOF(Parser *p, int ch) {
   return p->result = (ch == EOF) ? ParseDone : ParseFail;
}

ParserResult parse_NL2(Parser *p, int ch) {
   if (ch != '\n') return ParseFail;
   //return (p->state = parse_NL3(p, ch));
}

ParserResult parse_NL(Parser *p, int ch) {
   switch (p->substate) {
      case 0:
         if (parseWSP(p, ch) != ParseFail) break;
         if (ch != '\n') return p->result = ParseFail;
         p->substate++;
      case 2:
         if (parseWSP(p, ch) != ParseFail) return ParseOK;
         return ParseDone;
   }
   return p->result = ParseOK;
}
#endif

#if 0
bool Parser_consume(Parser *p, int ch) {
   p->ch = ch;
   if (p->state & Integer && (ch >= '0' && ch <= '9')) {
       p->integer *= 10;
       p->integer += ch;
       return true;
   }
   if (p->state & PLUSMINUS && (ch == '+' || ch == '-')) return true;
   if (p->state & DOT && (ch == '.')) return true;
   if (p->state & EXP && (ch == 'e' || ch == 'E')) return true;
   if (p->state & LWS) {
      if (ch == ' ' || ch == '\t' || ch == '\n') return true;
      p->state -= LWS;
   }
   if (p->state & WS) {
      if (ch == ' ' || ch == '\t') return true;
      p->state -= WS;
   }
   if (p->state & NL) {
      if (ch == '')
   }
   if (p->state & EOFT && ch == EOF) return true;
   return false;
}
#endif

void Parser_parse(Parser *p, int ch) {}

void Parser_construct(Parser *parser) {
   memset(parser, 0, sizeof(Parser));
   // parser->state = System;
   Node_construct(&parser->system, SystemNode, NULL);
}

void Parser_destruct(Parser *parser) {
   if (!parser) return;
   Node_destruct(&parser->system);
}

Parser *new_Parser() {
   Parser *parser = malloc(sizeof(Parser));
   Parser_construct(parser);
   return parser;
}

void delete_Parser(Parser *parser) {
   Parser_destruct(parser);
   free(parser);
}

int main() {}
