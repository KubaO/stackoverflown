#ifndef HEADER_H
#define HEADER_H

#include <QObject>

class Class : public QObject {
  Q_OBJECT
public:
  Q_SIGNAL void signal();
};

#endif // HEADER_H
