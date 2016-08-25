#ifndef LIB2_H
#define LIB2_H

#include "lib1/lib1.h"

#ifdef WIN32
#ifndef LIB2_EXPORT
#define LIB2_EXPORT __declspec(dllexport)
#endif
#else
#define LIB2_EXPORT
#endif

class LIB2_EXPORT Child : public Parent {
    Q_OBJECT
};

#endif
