#include "library.h"
#include <stdlib.h>

typedef struct callback_s {
   struct callback_s *next;
   library_callback function;
   void *parameter;
} callback;

static callback *cb_head;

void library_init(void) { /* some other code */
}
void library_deinit(void) { library_deregister_all_callbacks(); }

library_register_result library_register_callback(library_callback cb, void *cb_param) {
   callback *el = cb_head;
   while (el) {
      if (el->function == cb && el->parameter == cb_param) return LIBRARY_REG_DUPLICATE;
      el = el->next;
   }
   el = malloc(sizeof(callback));
   if (!el) return LIBRARY_REG_FAILURE;
   el->next = cb_head;
   el->function = cb;
   el->parameter = cb_param;
   cb_head = el;
   cb(LIBRARY_REGISTER, 0, &el->parameter);
   return LIBRARY_REG_SUCCESS;
}

static int match_callback(const callback *el, library_callback cb, void *cb_param) {
   return el && el->function == cb && el->parameter == cb_param;
}

static int match_any_callback(const callback *el, library_callback cb, void *cb_param) {
   return el && el->function == cb;
}

static int match_all_callbacks(const callback *el, library_callback cb, void *cb_param) {
   return !!el;
}

typedef int (*matcher)(const callback *, library_callback, void *);

static void deregister_callback(matcher match, library_callback cb, void *cb_param) {
   callback **p = &cb_head;
   while (*p) {
      callback *el = *p;
      if (match(el, cb, cb_param)) {
         *p = el->next;
         el->function(LIBRARY_DEREGISTER, 0, &el->parameter);
         free(el);
      } else
         p = &el->next;
   }
}

void library_deregister_callback(library_callback cb, void *cb_param) {
   deregister_callback(match_callback, cb, cb_param);
}

void library_deregister_any_callback(library_callback cb) {
   deregister_callback(match_any_callback, cb, NULL);
}

void library_deregister_all_callbacks(void) {
   deregister_callback(match_all_callbacks, NULL, NULL);
}

void library_sample(void) {
   int data = 42;
   // execute a bunch of code and then call the callback function
   callback *el = cb_head;
   while (el) {
      el->function(LIBRARY_SAMPLE, data, &el->parameter);
      el = el->next;
   }
}
