// https://github.com/KubaO/stackoverflown/tree/master/questions/demand-load-39291032
// ###main/main.cpp
#include "lib1/lib1.h"
#include <QtCore>

int main() {
    auto a = My_Add(1, 2);
    Q_ASSERT(a == 3);
    auto b = My_Add(3, 4);
    Q_ASSERT(b == 7);
    auto c = My_Subtract(5, 7);
    Q_ASSERT(c == -2);
}
