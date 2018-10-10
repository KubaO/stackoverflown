#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node_t Node;
typedef struct List_t List;
typedef struct Parser_t Parser;

// List of AST Nodes

struct List_t {
   size_t size;
   size_t capacity;
   Node **data;
};

void List_grow_(List *list, size_t capacity) {
   assert(list);
   if (capacity >= list->capacity)
      list->data = realloc(list->data, capacity * sizeof(struct Node_t *));
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

Node **List_find(List const *list, Node *item) {
   Node const *const *const end = list->data + list->size;
   for (Node const *const *p = list->data; p != end; p++)
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

struct Node_t *List_at(List *list, size_t i) {
   assert(list && i < list->size);
   return list->data[i];
}

// AST Node

typedef enum { MonomialNode, SumNode, EquationNode, SystemNode } NodeType;

struct Node_t {
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
rule_t r_EOFT[] = { -1, 0 };
rule_t r_WSP[]  = { -'*', -'(', ' ', -'/', '\t', -')', 0 };
rule_t r_NL[]   = { &r_WSP, '\n', &r_WSP, 0 };
rule_t r_LWS[]  = {};

typedef enum { // ABNF Syntax
   System   = /* LWS [Equation *(NL Equation)] LWS EOFT                 */ 1 << 1,
   Equation = /* Left "=" Right                                         */ 1 << 2,
   Left     = /* Sum                                                    */ 1 << 3,
   Right    = /* Sum                                                    */ 1 << 4,
   Sum      = /* *WSP Monomial *Term *WSP                               */ 1 << 5,
   Term     = /* *WSP Operator *WSP *Monomial                           */ 1 << 6,
   Operator = /* PLUSMINUS                                              */ 1 << 7,
   Monomial = /* Factor / Factor *WSP [TIMES] *WSP Variable / Variable  */ 1 << 8,
   Factor   = /* [Sign] *WSP Number                                     */ 1 << 9,
   Variable = /* ALPHA                                                  */ 1 << 10,
   Sign     = /* PLUSMINUS                                              */ 1 << 11,
   Number   = /* (Integer [DOT Integer] / DOT Integer) [EXP [Sign] Integer] */ 1 << 12,
   Integer  = /* 1*DIGIT                                                */ 1 << 13,
   PLUSMINUS = /* "+"/"-"                                               */ 1 << 14,
   DOT      = /* "."                                                    */ 1 << 15,
   EXP      = /* "e"                                                    */ 1 << 16,
   LWS      = /* *(WSP / LF)                                            */ 1 << 17,
   WS       = /* *WSP                                                   */ 1 << 18,
   NL       = /* *WSP LF *WSP                                           */ 1 << 19,
   EOFT     = /* EOF                                                    */ 1 << 20
} ParserState;

typedef enum {
   ParseDone,
   ParseOK,
   ParseFail
} ParserResult;

struct Parser_t {
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

ParserResult parse_EOF(Parser *p, int ch) {
   return p->result = (ch == EOF) ? ParseDone : ParseFail;
}

ParserResult parse_NL2(Parser *p, int ch) {
   if (ch != '\n') return ParseFail;
   return (p->state = parse_NL3(p, ch));
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

void Parser_parse(Parser *p, int ch) {
}

void Parser_construct(Parser *parser) {
   memset(parser, 0, sizeof(Parser));
   //parser->state = System;
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
