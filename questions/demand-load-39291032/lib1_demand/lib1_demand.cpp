// ###lib1_demand/lib1_demand.cpp
#define DEMAND_BUILD
#include "lib1/lib1.h"
#include <QLibrary>

void (* resolve_LIB1(const char * name))() {
    auto f = QLibrary::resolve("../lib1/liblib1", name);
    return f;
}
