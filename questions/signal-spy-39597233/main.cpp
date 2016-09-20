// https://github.com/KubaO/stackoverflown/tree/master/questions/signal-spy-39597233
#include <QtCore>
#include <private/qobject_p.h>

int signalToMethodIndex(const QMetaObject * mo, int signal)
{
    Q_ASSERT(signal >= 0);
    for (int i = 0; i < mo->methodCount(); ++i) {
        if (mo->method(i).methodType() == QMetaMethod::Signal) {
            if (signal == 0) return i;
            -- signal;
        }
    }
    return -1;
}

class Spy {
    static QThreadStorage<bool> entered;
    static void signalBegin(QObject *caller, int signalIndex, void **) {
        if (entered.localData()) return;
        QScopedValueRollback<bool> roll{entered.localData(), true};
        auto index = signalToMethodIndex(caller->metaObject(), signalIndex);
        if (index >= 0)
            qDebug() << "SIGNAL" << caller << caller->metaObject()->method(index).methodSignature();
    }
    static void slotBegin(QObject *caller, int index, void **) {
        if (entered.localData()) return;
        QScopedValueRollback<bool> roll{entered.localData(), true};
        qDebug() << "SLOT" << caller << caller->metaObject()->method(index).methodSignature();
    }
public:
   static void start() {
       QSignalSpyCallbackSet set{&signalBegin, &slotBegin, nullptr, nullptr};
       qt_signal_spy_callback_set = set;
   }
};
QThreadStorage<bool> Spy::entered;


struct Class : QObject {
    Q_SIGNAL void aSignal();
    Q_SLOT void aSlot() { qDebug() << "slot"; }
    Q_OBJECT
};

int main(int argc, char ** argv) {
    Spy::start();
    QCoreApplication app{argc, argv};
    Class obj;
    QObject::connect(&obj, SIGNAL(aSignal()), &obj, SLOT(aSlot()));
    obj.setObjectName("obj");
    emit obj.aSignal();
}
#include "main.moc"
