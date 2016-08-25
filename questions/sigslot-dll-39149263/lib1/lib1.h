//### lib1/lib1.h
#ifndef LIB1_H
#define LIB1_H

#include <QObject>

#ifdef WIN32
#ifndef LIB1_EXPORT
#define LIB1_EXPORT __declspec(dllimport)
#endif
#else
#define LIB1_EXPORT
#endif

class LIB1_EXPORT Parent : public QObject {
    Q_OBJECT
public:
    Q_SIGNAL void test();
};

#endif
