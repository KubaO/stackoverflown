#pragma once

#ifdef __cplusplus
extern "C" {
#pragma GCC diagnostic push
// clang erroneously issues a warning in spite of extern "C" linkage
#pragma GCC diagnostic ignored "-Wc++17-compat-mangling"
#endif

#ifndef LIBRARY_NOEXCEPT
#if __cplusplus >= 201103L
// c.f. https://stackoverflow.com/q/24362616/1329652
#define LIBRARY_NOEXCEPT noexcept
#else
#define LIBRARY_NOEXCEPT
#endif
#endif

enum library_register_enum { LIBRARY_REG_FAILURE = 0, LIBRARY_REG_SUCCESS = 1, LIBRARY_REG_DUPLICATE = -1 };
enum library_call_enum { LIBRARY_SAMPLE, LIBRARY_REGISTER, LIBRARY_DEREGISTER };
typedef enum library_register_enum library_register_result;
typedef enum library_call_enum library_call_type;
#if __cplusplus >= 201103L
void library_callback_dummy(library_call_type, int, void**) LIBRARY_NOEXCEPT;
using library_callback = decltype(&library_callback_dummy);
#else
typedef void (*library_callback)(library_call_type, int, void**);
#endif

void library_init(void) LIBRARY_NOEXCEPT;
library_register_result library_register_callback(library_callback cb, void *cb_param) LIBRARY_NOEXCEPT;
void library_deregister_callback(library_callback cb, void *cb_param) LIBRARY_NOEXCEPT;
void library_deregister_any_callback(library_callback cb) LIBRARY_NOEXCEPT;
void library_deregister_all_callbacks(void) LIBRARY_NOEXCEPT;
void library_deinit(void) LIBRARY_NOEXCEPT;

void library_sample(void) LIBRARY_NOEXCEPT;

#ifdef __cplusplus
#pragma GCC diagnostic pop
}
#endif
