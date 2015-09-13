#ifndef POSIXSIGNALPROXY_H
#define POSIXSIGNALPROXY_H

#include <QObject>

class PosixSignalProxy : public QObject
{
    Q_OBJECT
public:
    explicit PosixSignalProxy(QObject *parent = 0);

signals:

public slots:
};

#endif // POSIXSIGNALPROXY_H
