//main.cpp
#include <QtCore/QCoreApplication>
#include <QtCore/QThread>
#include <QtCore/QVector>
#include <QtCore/QDebug>
#include <QtCore/QAtomicInt>

class Class {
    static QAtomicInt m_copies;
    static QAtomicInt m_assignments;
    static QAtomicInt m_instances;
public:
    Class() { m_instances.fetchAndAddOrdered(1); }
    Class(const Class &) { m_copies.fetchAndAddOrdered(1); }
    Class & operator=(const Class &) { m_assignments.fetchAndAddOrdered(1); return *this; }
    static void dump(const QString & s = QString()) {
        qDebug() << s << "copies:" << m_copies << "assignments:" << m_assignments << "default instances:" << m_instances;
    }
    static void reset() {
        m_copies = 0;
        m_assignments = 0;
        m_instances = 0;
    }
};

QAtomicInt Class::m_instances;
QAtomicInt Class::m_copies;
QAtomicInt Class::m_assignments;

typedef QVector<Class> Vector;

Q_DECLARE_METATYPE(Vector)

class Foo : public QObject
{
    Q_OBJECT
    Vector v;
public:
    Foo() : v(100) {}
signals:
    void containerSignal(const Vector &);
    void classSignal(const Class &);
public slots:
    void sendContainer() { emit containerSignal(v); }
    void sendClass() { emit classSignal(Class()); }
};

class Bar : public QObject
{
    Q_OBJECT
public:
    Bar() {}
signals:
    void containerDone();
    void classDone();
public slots:
    void containerSlotConst(const Vector &) {
        Class::dump("Received signal w/const container");
    }
    void containerSlot(Vector v) {
        Class::dump("Received signal w/copy of the container");
        v[99] = Class();
        Class::dump("Made a copy");
        Class::reset();
        Class::dump("Reset");
        emit containerDone();
    }
    void classSlotConst(const Class &) {
        Class::dump("Received signal w/const class");
    }
    void classSlot(Class) {
        Class::dump("Received signal w/copy of the class");
        emit classDone();
        //QThread::currentThread()->quit();
    }
};

int main(int argc, char ** argv)
{
    QCoreApplication a(argc, argv);
    qRegisterMetaType<Vector>("Vector");
    qRegisterMetaType<Class>("Class");

    Class::dump("Started");
    QThread thread;
    Foo foo;
    Bar bar;
    Class::dump("Created Foo");
    bar.moveToThread(&thread);
    Class::dump("Created Bar");
    QObject::connect(&thread, SIGNAL(started()), &foo, SLOT(sendContainer()));
    QObject::connect(&foo, SIGNAL(containerSignal(Vector)), &bar, SLOT(containerSlotConst(Vector)));
    QObject::connect(&foo, SIGNAL(containerSignal(Vector)), &bar, SLOT(containerSlot(Vector)));
    QObject::connect(&bar, SIGNAL(containerDone()), &foo, SLOT(sendClass()));
    QObject::connect(&foo, SIGNAL(classSignal(Class)), &bar, SLOT(classSlotConst(Class)));
    QObject::connect(&foo, SIGNAL(classSignal(Class)), &bar, SLOT(classSlot(Class)));
    QObject::connect(&bar, SIGNAL(classDone()), &thread, SLOT(quit()));
    QObject::connect(&thread, SIGNAL(finished()), &a, SLOT(quit()));
    thread.start();
    a.exec();
    thread.wait();
}

#include "main.moc"
