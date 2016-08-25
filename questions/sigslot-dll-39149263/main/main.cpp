// https://github.com/KubaO/stackoverflown/tree/master/questions/sigslot-dll-39149263
#include "lib1/lib1.h"
#include "lib2/lib2.h"

int main() {
    int counter = 0;
    Child child;
    Parent * parent = &child;
    QObject::connect(parent, &Parent::test, [&]{ counter++; });
    emit parent->test();
    emit parent->test();
    Q_ASSERT(counter == 2);
}
