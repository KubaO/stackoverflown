// https://github.com/KubaO/stackoverflown/tree/master/questions/c-linked-debug-52867729
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
   int power;  // primary key for sorting
   int coeff;
   struct Node *next;
} Node;

Node *new_node(Node *prev, Node *next, int power);
Node *get_node(Node **head, Node *hint, int power);
void delete_nodes(Node *head);

Node *read(void);
void print(const Node *head);
Node *add(const Node *head1, const Node *head2);
Node *multiply(const Node *head1, const Node *head2);

void print_nodes(const Node *n1, const Node *n2, const char *extra_label,
                 const Node *extra);
const char *arity_suffix(int n);
bool parse_line(int max_length, const char *fmt, int count, ...);

int main() {
   Node *h1 = NULL, *h2 = NULL;
   int option;
   do {
      printf("\n1 : Create 1'st polynomial");
      printf("\n2 : Create 2'nd polynomial");
      printf("\n3 : Print polynomials");
      printf("\n4 : Add polynomials");
      printf("\n5 : Multiply polynomials");
      printf("\n6 : Quit");
      printf("\nEnter your choice: ");
      if (!parse_line(10, "%d", 1, &option)) continue;
      switch (option) {
         case 1:
            delete_nodes(h1);
            h1 = read();
            break;

         case 2:
            delete_nodes(h2);
            h2 = read();
            break;

         case 3:
            print_nodes(h1, h2, NULL, NULL);
            break;

         case 4: {
            Node *sum = add(h1, h2);
            print_nodes(h1, h2, "Sum", sum);
            delete_nodes(sum);
            break;
         }
         case 5: {
            Node *prod = multiply(h1, h2);
            print_nodes(h1, h2, "Product", prod);
            delete_nodes(prod);
            break;
         }
      }
   } while (option != 6);
   delete_nodes(h1);
   delete_nodes(h2);
}

Node *read() {
   int n;
   printf("\n Enter number of terms: ");
   if (!parse_line(10, "%d", 1, &n)) return NULL;
   /* read n terms */
   Node *head = NULL;
   for (int i = 0; i < n;) {
      int power, coeff;
      printf("\nEnter the %d%s term (power coeff): ", i + 1, arity_suffix(i + 1));
      if (!parse_line(80, "%d%d", 2, &power, &coeff) || !coeff) continue;
      Node *p = get_node(&head, NULL, power);
      if (!p->coeff) i++;  // count only new terms
      p->coeff = coeff;
   }
   return head;
}

void print(const Node *p) {
   for (; p; p = p->next) printf("%dX^%d ", p->coeff, p->power);
}

void add_to(Node **sum, const Node *h) {
   Node *r = NULL;
   for (; h; h = h->next) {
      r = get_node(sum, r, h->power);
      r->coeff += h->coeff;
   }
}

Node *add(const Node *h1, const Node *h2) {
   Node *sum = NULL;
   add_to(&sum, h1);
   add_to(&sum, h2);
   return sum;
}

Node *multiply(const Node *h1, const Node *h2) {
   Node *prod = NULL;
   for (const Node *p = h1; p; p = p->next) {
      Node *r = NULL;
      for (const Node *q = h2; q; q = q->next) {
         int power = p->power + q->power;
         r = get_node(&prod, r, power);
         r->coeff += p->coeff * q->coeff;
      }
   }
   return prod;
}

Node *new_node(Node *prev, Node *next, int power) {
   assert(!prev || prev->power < power);
   assert(!next || next->power > power);
   Node *node = malloc(sizeof(Node));
   node->power = power;
   node->coeff = 0;
   node->next = next;
   if (prev) prev->next = node;
   return node;
}

void delete_nodes(Node *head) {
   while (head) {
      Node *p = head;
      head = head->next;
      free(p);
   }
}

static bool list_contains(Node *head, Node *elt) {
   for (; head; head = head->next)
      if (head == elt) return true;
   return false;
}

Node *get_node(Node **head, Node *hint, int power) {
   Node *node = hint;
   Node *next = hint ? hint->next : head ? *head : NULL;
   assert(!hint || !*head || list_contains(*head, hint));
   assert(!hint || hint->power <= power);
   assert(!node || !next || node->power < next->power);

   while (next && next->power <= power) {
      node = next;
      next = next->next;
   }
   if (!node || node->power != power) {
      assert(!node || node->power < power);
      Node *n = new_node(node, next, power);
      if (!node) *head = n;
      node = n;
   }
   return node;
}

void print_nodes(const Node *h1, const Node *h2, const char *extra_label,
                 const Node *extra) {
   printf("\n1'st polynomial -> ");
   print(h1);
   printf("\n2'nd polynomial -> ");
   print(h2);
   if (extra_label) {
      printf("\n %s = ", extra_label);
      print(extra);
   }
   printf("\n");
}

const char *arity_suffix(int n) {
   if (n == 0) return "st";
   if (n == 1) return "nd";
   return "rd";
}

bool parse_line(int max_length, const char *fmt, int count, ...) {
   bool result = false;
   int const buf_size = max_length + 2;  // include \n and zero termination
   char *const buf = malloc((size_t)buf_size);
   char *const str = fgets(buf, buf_size, stdin);
   if (str) {
      size_t n = strlen(str);
      if (str[n - 1] == '\n') {  // we must have read a whole line
         str[n - 1] = '\0';      // remove the newline
         va_list ap;
         va_start(ap, count);
         int rc = vsscanf(buf, fmt, ap);
         va_end(ap);
         result = rc == count;
      }
   }
   free(buf);
   return result;
}
