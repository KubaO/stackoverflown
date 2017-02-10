// https://github.com/KubaO/stackoverflown/tree/master/questions/const-slot-arg-42163294
#include <QtCore>

struct ProcessHandle {};

struct Object : QObject {
    int counter = 0;
    Q_SIGNAL void newValue(const ProcessHandle*, int val);
    Q_SLOT void useValue(const ProcessHandle* const ph, const int val) {
        qDebug() << ph << val;
        Q_ASSERT(ph == nullptr && val == 42);
        ++counter;
    }
    Q_OBJECT
};

int main(int argc, char ** argv) {
    QCoreApplication app{argc, argv};
    Object obj;
    QObject::connect(&obj, &Object::newValue, &obj, &Object::useValue, Qt::QueuedConnection);
    QObject::connect(&obj, &Object::newValue, &app, &QCoreApplication::quit, Qt::QueuedConnection);
    emit obj.newValue(nullptr, 42);
    app.exec();
    Q_ASSERT(obj.counter == 1);
}

#include "main.moc"
