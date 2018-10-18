// https://github.com/KubaO/stackoverflown/tree/master/questions/c-linked-debug-52867729
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 17

typedef struct Node {
   int coeff;
   struct Node *next;
} Node;

Node *new_nodes(void);
void clear_nodes(Node *head);
void delete_nodes(const Node *head);
Node *new_or_clear_nodes(Node *head);
Node *get_node(Node *head, int i);
void read(Node *head);
void print(const Node *head);
Node *add(const Node *head1, const Node *head2);
Node *multiply(const Node *head1, const Node *head2);

void print_nodes(const Node *n1, const Node *n2, const char *extra_label, const Node *extra);
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
            h1 = new_or_clear_nodes(h1);
            read(h1);
            break;

         case 2:
            h2 = new_or_clear_nodes(h2);
            read(h2);
            break;

         case 3:
            print_nodes(h1, h2, NULL, NULL);
            break;

         case 4: {
            const Node *sum = add(h1, h2);
            print_nodes(h1, h2, "Sum", sum);
            delete_nodes(sum);
            break;
         }
         case 5: {
            const Node *prod = multiply(h1, h2);
            print_nodes(h1, h2, "Product", prod);
            delete_nodes(prod);
            break;
         }
      }
   } while (option != 6);
   delete_nodes(h1);
   delete_nodes(h2);
}

void read(Node *h) {
   int n;
   printf("\n Enter number of terms: ");
   if (!parse_line(10, "%d", 1, &n))
      return;
   /* read n terms */
   for (int i = 0; i < n;) {
      int power, coeff;
      printf("\nEnter the %d%s term (power coeff): ", i+1, arity_suffix(i+1));
      if (!parse_line(80, "%d%d", 2, &power, &coeff))
         continue;
      Node *p = get_node(h, power);
      if (p) {
         p->coeff += coeff;
         i ++;
      }
   }
}

void print(const Node *p) {
   for (int i = 0; p; i++, p = p->next)
      if (p->coeff != 0) printf("%dX^%d ", p->coeff, i);
}

Node *add(const Node *h1, const Node *h2) {
   Node *const sum = new_nodes();
   Node *p = sum;
   while (h1 && h2) {
      p->coeff = h1->coeff + h2->coeff;
      h1 = h1->next;
      h2 = h2->next;
      p = p->next;
   }
   return sum;
}

Node *multiply(const Node *h1, const Node *h2) {
   Node *const prod = new_nodes();
   const Node *p;
   int i;
   for (p = h1, i = 0; p; p = p->next, i++) {
      const Node *q;
      int j;
      for (q = h2, j = 0; q; q = q->next, j++) {
         int const coeff = p->coeff * q->coeff;
         int power = i + j;
         Node *r = get_node(prod, power);
         assert(r);
         r->coeff = r->coeff + coeff;
      }
   }
   return prod;
}

static Node *new_node(Node *next) {
   Node *p = malloc(sizeof(Node));
   p->coeff = 0;
   p->next = next;
   return p;
}

Node *new_nodes(void) {
   Node *p = NULL;
   for (int i = 0; i < MAX; i++)
      p = new_node(p);
   return p;
}

void delete_nodes(const Node *p) {
   while (p) {
      const Node *head = p;
      p = p->next;
      free((Node*)head);
   }
}

void clear_nodes(Node *p) {
   for (; p; p = p->next)
      p->coeff = 0;
}

Node *new_or_clear_nodes(Node *p) {
   if (p) clear_nodes(p); else p = new_nodes();
   return p;
}

Node *get_node(Node *node, int n) {
   while (node && n) {
      node = node->next;
      n--;
   }
   return node;
}

void print_nodes(const Node *h1, const Node *h2,
                 const char *extra_label, const Node *extra) {
   printf("\n1'st polynomial -> ");
   print(h1);
   printf("\n2'nd polynomial -> ");
   print(h2);
   if (extra_label && extra) {
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
   int const buf_size = max_length + 2; // include \n and zero termination
   char *const buf = malloc(buf_size);
   char *const str = fgets(buf, buf_size, stdin);
   if (str) {
      size_t n = strlen(str);
      if (str[n-1] == '\n') { // we must have read a whole line
         str[n-1] = '\0'; // remove the newline
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
