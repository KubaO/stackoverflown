// śmyobject.h
#ifndef MYOBJECT_H
#define MYOBJECT_H
#include <QObject>

class MyObject : public QObject {
   Q_OBJECT
public:
   MyObject() {}
   Q_SLOT void aSlot() {}
};

#endif // MYOBJECT_H
