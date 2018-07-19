#include "header.h"

struct Foo {};

int main() {
   Class c;
   Foo d;
   Q_UNUSED((d = {}));
}

#include "moc_header.cpp"
