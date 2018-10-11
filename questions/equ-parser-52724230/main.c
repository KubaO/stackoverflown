#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(var) ((void)(var))
typedef struct List List;
typedef struct Group Group;
typedef struct Node Node;
typedef struct Parser Parser;
typedef intptr_t Rule;
inline size_t sizeof_Node(void);
inline size_t sizeof_Group(void);

// Generic List

struct List {
   size_t size;
   size_t capacity;
   size_t elementSize;
   void **data;
};

void List_grow_(List *list, size_t capacity) {
   assert(list);
   if (capacity >= list->capacity)
      list->data = realloc(list->data, capacity * sizeof(void *));
}

#if 0
List *new_List(size_t elementSize, size_t capacity) {
   List *list = malloc(sizeof(List));
   List_construct(list, elementSize, capacity);
   return list;
}

void delete_List(List *list) {
   List_destruct(list);
   free(list);
}
#endif

size_t List_size(List *list) { return list->size; }

#define DECLARE_LIST_FOR(Type)                                               \
   void List_construct_##Type(List *list, size_t capacity) {                 \
      memset(list, 0, sizeof(List));                                         \
      list->elementSize = sizeof_##Type();                                   \
      List_grow_(list, capacity);                                            \
   }                                                                         \
   Type **List_find_##Type(List *list, Type *item) {                         \
      assert(list->elementSize == sizeof_##Type());                          \
      void **const end = list->data + list->size;                            \
      for (void **p = list->data; p != end; p++)                             \
         if (*p == item) return p;                                           \
      return NULL;                                                           \
   }                                                                         \
   void List_push_back_##Type(List *list, Type *item) {                      \
      assert(list->elementSize == sizeof_##Type());                          \
      assert(!List_find_##Type(list, item));                                 \
      if (list->size >= list->capacity)                                      \
         List_grow_(list, list->capacity ? 2 * list->capacity : 1);          \
      list->data[list->size++] = item;                                       \
   }                                                                         \
   Type *List_pop_back_##Type(List *list) {                                  \
      assert(list->elementSize == sizeof_##Type());                          \
      assert(list->size > 0);                                                \
      return list->data[--list->size];                                       \
   }                                                                         \
   Type *List_at_##Type(List *list, size_t i) {                              \
      assert(list->elementSize == sizeof_##Type());                          \
      assert(list &&i < list->size);                                         \
      return list->data[i];                                                  \
   }                                                                         \
   Type *List_back_##Type(List *list) {                                      \
      assert(list->elementSize == sizeof_##Type());                          \
      assert(list && list->size);                                            \
      return list->data[list->size - 1];                                     \
   }                                                                         \
   bool List_remove_##Type(List *list, Type *item) {                         \
      assert(list->elementSize == sizeof_##Type());                          \
      size_t i = list->size;                                                 \
      while (i--)                                                            \
         if (list->data[i] == item) {                                        \
            memmove(list->data + i, list->data + i + 1, list->size - i - 1); \
            list->size--;                                                    \
            return true;                                                     \
         }                                                                   \
      return false;                                                          \
   }                                                                         \
   void Type##_destruct(Type *);                                             \
   void List_destruct_##Type(List *list) {                                   \
      if (!list) return;                                                     \
      assert(list->elementSize == sizeof_##Type());                          \
      size_t size = List_size(list);                                         \
      while (size) Type##_destruct(List_at_##Type(list, --size));            \
      free(list->data);                                                      \
   }

DECLARE_LIST_FOR(Node)
DECLARE_LIST_FOR(Group)

// ABNF Rule Group

struct Group {
   const Rule *begin, *end;
   size_t minReps, maxReps;
   size_t curReps;
};

inline size_t sizeof_Group(void) { return sizeof(Group); }

void Group_construct(Group *group, const Rule *begin) {
   memset(group, 0, sizeof(Group));
   group->begin = begin;
   group->maxReps = SIZE_MAX;
}

void Group_destruct(Group *group) { UNUSED(group); }

Group *new_Group(const Rule *begin) {
   Group *group = malloc(sizeof(Group));
   Group_construct(group, begin);
   return group;
}

void delete_Group(Group *group) {
   Group_destruct(group);
   free(group);
}

// Syntax Declarations

typedef struct RuleName RuleName;
struct RuleName {
   const Rule *rule;
   const char *name;
};

#define RULES     \
   RULE(System)   \
   RULE(Equation) \
   RULE(Sum)      \
   RULE(Term)     \
   RULE(Monomial) \
   RULE(Equals)   \
   RULE(Operator) \
   RULE(Factor)   \
   RULE(Variable) \
   RULE(Number)   \
   RULE(Exponent) \
   RULE(Sign)     \
   RULE(Integer)  \
   RULE(DIGIT) RULE(PLUSMINUS) RULE(WS) RULE(LWS) RULE(NL) RULE(WSP) RULE(EOF) RULE(LF)
#define RULE(name) extern const Rule name##_[];
RULES
#undef RULE
#define RULE(name_) {&name_##_[0], #name_},
static RuleName ruleNames[] = {RULES};
#undef RULE
#undef RULES

// AST Node

struct Node {
   const Rule *firstRule, *rule;
   Node *parent;
   List children;
   bool keep;
   double factor;
   char variable;
};

inline size_t sizeof_Node(void) { return sizeof(Node); }

void Node_construct(Node *node, const Rule *firstRule, Node *parent) {
   memset(node, 0, sizeof(Node));
   node->firstRule = firstRule;
   node->rule = firstRule;
   node->parent = parent;
   List_construct_Node(&node->children, 0);
   if (parent) List_push_back_Node(&parent->children, node);
}

void Node_destruct(Node *node) {
   if (!node) return;
   if (node->parent) List_remove_Node(&node->parent->children, node);
   List_destruct_Node(&node->children);
}

Node *new_Node(const Rule *firstRule, Node *parent) {
   Node *node = malloc(sizeof(Node));
   Node_construct(node, firstRule, parent);
   return node;
}

void delete_Node(Node *node) {
   Node_destruct(node);
   free(node);
}

// Syntax Definition

// ABNF Syntax with Extensions
// -'c': literal "c"
// '^', foo: literal foo value
// '!': keep the node, otherwise the node is discarded from the AST in a cleanup pass
// clang-format off
const Rule
   System_[]      = { '!', LWS_, '[', Equation_, '*', '(', NL_, Equation_, ')', ']', LWS_, EOF_, 0 },
   Equation_[]    = { '!', Sum_, Equals_, Sum_, 0 },
   Sum_[]         = { Monomial_, '*', Term_, WS_, 0 },
   Term_[]        = { Operator_, Monomial_, 0 },
   Monomial_[]    = { '!', Factor_, '[', Variable_, ']', '/', Variable_, 0 },
   Equals_[]      = { WS_, -'=', 0 },
   Operator_[]    = { WS_, PLUSMINUS_, 0 },
   Factor_[]      = { WS_, '[', Sign_, ']', WS_, Number_, 0 },
   Variable_[]    = { WS_, -'a', '-', -'z', 0 },
   Number_[]      = { WS_, '(', Integer_, '[', -'.', Integer_, ']', '/', -'.', Integer_, ')', '[', Exponent_, ']', 0 },
   Exponent_[]    = { -'e', '[', Sign_, ']', Integer_, ']', 0 },
   Sign_[]        = { PLUSMINUS_, 0 },
   Integer_[]     = { '1', '*', DIGIT_, 0 },
   DIGIT_[]       = { -'0', '-', -'9', 0 },
   PLUSMINUS_[]   = { -'+', '/', -'-', 0 },
   WS_[]          = { '*', WSP_, 0 },
   LWS_[]         = { '*', '(', WSP_, '/', LF_, ')', 0 },
   NL_[]          = { WS_, LF_, WS_, 0 },
   WSP_[]         = { -' ', '/', -'\t', 0 },
   EOF_[]         = { '^', EOF, 0 },
   LF_[]          = { -'\n', 0 };
// clang-format on

struct Parser {
   Node ast;
   Node *top;
   int ch;
   List groups;
};

bool Parser_error(Parser *p, const char *msg) { return false; }

bool Parser_parse(Parser *p, int ch) {
   while (true) {
      Node *const top = p->top;
      if (!top) return ch ? Parser_error(p, "Unmatched character") : true;
      Node *const parent = top->parent;
      const Rule rule = *top->rule++;
      if (rule == 0) {  // exit node
         if (!top->keep) delete_Node(top);
         p->top = parent;
      } else if (rule > 255 || rule < -255) {  // enter new node
         p->top = new_Node((const Rule *)rule, top);
      } else if (rule == '!') {
         top->keep = true;
      } else if (rule == '[') {
         // List_push_back
      }

      else if (rule >= -255 && rule < 0) {  // literal
      }
   }
}

void Parser_construct(Parser *parser) {
   memset(parser, 0, sizeof(Parser));
   Node_construct(&parser->ast, &System_[0], NULL);
   parser->top = &parser->ast;
   List_construct_Group(&parser->groups, 10);
   Group *group = new_Group(parser->ast.firstRule);
   List_push_back_Group(&parser->groups, group);
   group->minReps = 1;
   group->maxReps = 1;
}

void Parser_destruct(Parser *parser) {
   if (!parser) return;
   Node_destruct(&parser->ast);
   List_destruct_Group(&parser->groups);
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

int main() {
   Parser parser;
   Parser_construct(&parser);


   Parser_destruct(&parser);
}
