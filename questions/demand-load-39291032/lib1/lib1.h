// ###lib1/lib1.h
#ifndef LIB_H
#define LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#define DEMAND_NAME LIB1
#ifdef DEMAND_LOAD_LIB1
#define DEMAND_LOAD
#endif
#include "demand_load.h"
#undef DEMAND_LOAD

DEMAND_FUN(int, My_Add, (int i, int j), (i,j))
DEMAND_FUN(int, My_Subtract, (int i, int j), (i,j))

#undef DEMAND_FUN
#undef DEMAND_NAME

#ifdef __cplusplus
}
#endif

#endif
