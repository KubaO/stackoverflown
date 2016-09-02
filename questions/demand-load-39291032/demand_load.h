// ###demand_load.h
// Configuration macros:
// DEMAND_NAME - must be set to a unique identifier of the library
// DEMAND_LOAD - if defined, the functions are declared as function pointers, **or**
// DEMAND_BUILD - if defined, the thunks and function pointers are defined

#if defined(DEMAND_FUN)
#error Multiple inclusion of demand_load.h without undefining DEMAND_FUN first.
#endif

#if !defined(DEMAND_NAME)
#error DEMAND_NAME must be defined
#endif

#if defined(DEMAND_LOAD)
// Interface via a function pointer
#define DEMAND_FUN(ret,name,args,arg_call) \
    extern ret (*name)args;

#elif defined(DEMAND_BUILD)
// Implementation of the demand loader stub
#ifndef DEMAND_CAT
#define DEMAND_CAT_(x,y) x##y
#define DEMAND_CAT(x,y) DEMAND_CAT_(x,y)
#endif
void (* DEMAND_CAT(resolve_,DEMAND_NAME)(const char *))();
#if defined(__cplusplus)
#define DEMAND_FUN(ret,name,args,arg_call) \
    extern ret (*name)args; \
    ret name##_thunk args { \
        name = reinterpret_cast<decltype(name)>(DEMAND_CAT(resolve_,DEMAND_NAME)(#name)); \
        return name arg_call; \
    }\
    ret (*name)args = name##_thunk;
#else
#define DEMAND_FUN(ret,name,args,arg_call) \
    extern ret (*name)args; \
    ret name##_impl args { \
        name = (void*)DEMAND_CAT(resolve_,DEMAND_NAME)(#name); \
        name arg_call; \
    }\
    ret (*name)args = name##_impl;
#endif // __cplusplus

#else
// Interface via a function
#define DEMAND_FUN(ret,name,args,arg_call) \
    ret name args;
#endif
