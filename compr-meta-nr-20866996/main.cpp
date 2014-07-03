#include <QScopedPointer>
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QFormLayout>
#include <QSet>
#include <QMetaMethod>
#include <QThread>
#include <QPointer>
#include <QBasicTimer>
#include <QDebug>

/*! A type-identifier-generating wrapper for events.
 * See http://stackoverflow.com/a/19189941/1329652 for discussion */
template <typename Derived> class EventWrapper : public QEvent {
public:
    EventWrapper() : QEvent(staticType()) {}
    static QEvent::Type staticType() {
        static QEvent::Type type = static_cast<QEvent::Type>(registerEventType());
        return type;
    }
    static bool is(const QEvent * ev) { return ev->type() == staticType(); }
    static Derived* cast(QEvent * ev) { return is(ev) ? static_cast<Derived*>(ev) : 0; }
};

/*! A metacall. */
class MetaCall {
    Q_DISABLE_COPY(MetaCall)
public:
    QObject * const m_sender;
    int const m_signal;
    int const m_method;
private:
    int m_nargs;
    void ** m_args;
public:
    template <typename iter>
    MetaCall(QObject * sender, int signal, int method, iter arguments, iter argumentsEnd, void ** argv) :
        m_sender(sender), m_signal(signal), m_method(method),
        m_nargs(1 + (argumentsEnd - arguments)), // include return type
        m_args((void**) malloc(m_nargs*(sizeof(void*) + sizeof(int))))
    {
        Q_CHECK_PTR(m_args);
        int *types = (int *)(m_args + m_nargs);
        types[0] = 0; // return type
        m_args[0] = 0; // return value
        // Copy the arguments and queue the call
        for (int i = 1; i < m_nargs; ++i)
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
            m_args[i] = QMetaType::create((types[i] = *(arguments++)), argv[i]);
#else
            m_args[i] = QMetaType::construct((types[i] = *(arguments++)), argv[i]);
#endif
    }
    ~MetaCall() {
        if (! m_args) return;
        int * types = (int*)(m_args + m_nargs);
        for (int i = 0; i < m_nargs; ++i) {
            if (types[i] && m_args[i])
                QMetaType::destroy(types[i], m_args[i]);
        }
        free(m_args);
    }
    void placeCall(QObject *object) {
        QMetaObject::metacall(object, QMetaObject::InvokeMetaMethod, m_method, m_args);
    }
};

/*! A meta call event, carrying detachable meta call. */
class MetaCallEvent : public EventWrapper<MetaCallEvent> {
    QScopedPointer<class MetaCall> m_call;
public:
    explicit MetaCallEvent(class MetaCall * call) : m_call(call) {}
    class MetaCall * take() { return m_call.take(); }
    const class MetaCall & operator->() const { return *m_call.data(); }
};

/*! Notifies the target object that a compressed connection is to be added.  */
class ConnectEvent : public EventWrapper<ConnectEvent> {
    int m_signal;
    QPointer<QObject> m_receiver;
    int m_method;
public:
    explicit ConnectEvent(int signal, QObject * receiver, int method) :
        m_signal(signal), m_receiver(receiver), m_method(method) {}
    inline int signal() const { return m_signal; }
    inline const QPointer<QObject> & receiver() const { return m_receiver; }
    inline int method() const { return m_method; }
};

/*! Represents a compressed connection to a target object. */
struct CompressedConnection {
    bool valid;
    QVector<int> argumentTypes;
    int signal;
    QPointer<QObject> target;
    int method;
    CompressedConnection() : valid(false) {}
    CompressedConnection(QObject * sender, int signal_, const QPointer<QObject> & target_, int method_) :
        valid(false), signal(signal_), target(target_), method(method_)
    {
        QMetaMethod sig = sender->metaObject()->method(signal_);
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        int nparams = sig.parameterCount();
        argumentTypes.resize(nparams);
        valid = true;
        for (int i = 0; i < nparams; ++i) {
            if ((argumentTypes[i] = sig.parameterType(i)) == QMetaType::UnknownType) valid = false;
        }
#else
        const QList<QByteArray> & types = sig.parameterTypes();
        int nparams = types.count();
        argumentTypes.resize(nparams);
        valid = true;
        for (int i = 0; i < nparams; ++i) {
            int type = QMetaType::type(types[i].constData());
            if ((argumentTypes[i] = type) == 0) valid = false;
        }
#endif
    }
};

/*! Compressor proxy attached to each sender. */
class CompressProxy : public QObject {
    int m_endIndex;
    QList<CompressedConnection> m_connections;
    int qt_metacall(QMetaObject::Call call, int id, void ** argv) {
        if (id < m_endIndex) return QObject::qt_metacall(call, id, argv);
        id -= m_endIndex;
        if (id > m_connections.size()) return 0;
        const CompressedConnection & conn = m_connections.at(id);
        qDebug() << "got metacall [" << id << "] for" << conn.target.data();
        QScopedPointer<MetaCall> mc(new MetaCall(parent(), conn.signal, conn.method,
                                                 conn.argumentTypes.begin(), conn.argumentTypes.end(), argv));
        QCoreApplication::postEvent(conn.target, new MetaCallEvent(mc.take()));
        return -1;
    }
    void customEvent(QEvent * ev) {
        if (ev->type() != ConnectEvent::staticType()) return;
        QObject * sender = parent();
        ConnectEvent * cev = static_cast<ConnectEvent*>(ev);
        const CompressedConnection & conn = CompressedConnection(sender, cev->signal(), cev->receiver(), cev->method());
        if (conn.valid) {
            m_connections << conn;
            QMetaObject::connect(sender, cev->signal(), this, m_endIndex + m_connections.size() - 1);
        }
    }
public:
    explicit CompressProxy(QObject * parent) : QObject(parent),
        m_endIndex(staticMetaObject.methodOffset() + staticMetaObject.methodCount()) {}
};

// A per-thread proxy exists.
// Compression is done via a custom connection to the *proxy* object. Setup:
// 1. The proxy installs itself as an event filter on the target.
// 2. A proxy

// registered with the proxy.
// Compression is done via a custom connection to the target object. The connection uses a non-existent slot.
// The proxy is per-thread, it is only used to

// The proxy is per-thread. It's used to filter metacall and call flush events.
// 1. Receive a metacall event.
// 2. Check if a call flush flag is set on the object.
//    If not, post a call flush event and set the flag. That event brackets the event queue.
// 3. In a per-object map in the userdata, retain a copy of the event data.
// ...
// 1. Receive a call flush event.
// 2. Execute the queued calls.

/*! A compressor application that processes ConnectEvent and MetaCallEvent deliveries. */
template <class Base> class CompressorApplication : public Base {
public:
    CompressorApplication(int & argc, char ** argv) : Base(argc, argv) {}
protected:
    bool notify(QObject * receiver, QEvent * event) {
        if (event->type() == ConnectEvent::staticType())  {
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
            CompressProxy * proxy = receiver->findChild<CompressProxy*>(QString(), Qt::FindDirectChildrenOnly);
#else
            CompressProxy * proxy = receiver->findChild<CompressProxy*>();
#endif
            if (! proxy) proxy = new CompressProxy(receiver);
            return Base::notify(proxy, event);
        }
        else if (event->type() == MetaCallEvent::staticType()) {
            MetaCallEvent * mc = static_cast<MetaCallEvent*>(event);
            QScopedPointer<MetaCall> call(mc->take());
            call->placeCall(receiver);
        }
        return Base::notify(receiver, event);
    }
};

bool compressedConnect(QObject * sender, const char * signalRaw, QObject * receiver, const char * methodRaw) {
    if (signalRaw[0] != '0'+QSIGNAL_CODE ||
        (methodRaw[0] != '0'+QSIGNAL_CODE && methodRaw[0] != '0'+QSLOT_CODE && methodRaw[0] != '0'+QMETHOD_CODE)) return false;
    QByteArray signal = QMetaObject::normalizedSignature(signalRaw + 1);
    QByteArray method = QMetaObject::normalizedSignature(methodRaw + 1);
    if (! QMetaObject::checkConnectArgs(signal, method)) return false;
    int signalIndex = sender->metaObject()->indexOfSignal(signal);
    int methodIndex = receiver->metaObject()->indexOfMethod(method);
    if (signalIndex < 0 || methodIndex < 0) return false;
    if (sender->thread() == QThread::currentThread()) {
        ConnectEvent event(signalIndex, receiver, methodIndex);
        QCoreApplication::sendEvent(sender, &event);
    } else {
        QCoreApplication::postEvent(sender, new ConnectEvent(signalIndex, receiver, methodIndex));
    }
    return true;
}

//
// Demo GUI

class Signaller : public QObject {
    Q_OBJECT
public:
    Q_SIGNAL void emptySignal();
    Q_SIGNAL void dataSignal(int);
};

class Widget : public QWidget {
    Q_OBJECT
    QBasicTimer m_timer;
    QPlainTextEdit * m_edit;
    QSpinBox * m_count;
    Signaller m_signaller;
    Q_SLOT void emptySlot() {
        m_edit->appendPlainText("emptySlot invoked");
        qDebug() << "empty slot";
        m_timer.start(0, this);
    }
    Q_SLOT void dataSlot(int n) {
        m_edit->appendPlainText(QString("dataSlot(%1) invoked").arg(n));
    }
    Q_SLOT void sendSignals() {
        m_edit->appendPlainText(QString("\nEmitting %1 signals").arg(m_count->value()));
        for (int i = 0; i < m_count->value(); ++ i) {
            emit m_signaller.emptySignal();
            emit m_signaller.dataSignal(i + 1);
        }
    }
    void timerEvent(QTimerEvent * ev) {
        if (ev->timerId() == m_timer.timerId()) {
            qDebug() << "timer 0";
            m_timer.stop();
        }
    }
public:
    Widget(QWidget * parent = 0) : QWidget(parent),
        m_edit(new QPlainTextEdit), m_count(new QSpinBox)
    {
        QFormLayout * l = new QFormLayout(this);
        QPushButton * invoke = new QPushButton("Invoke");
        m_edit->setReadOnly(true);
        m_count->setRange(1, 1000);
        l->addRow("Number of slot invocations", m_count);
        l->addRow(invoke);
        l->addRow(m_edit);
        connect(invoke, SIGNAL(clicked()), SLOT(sendSignals()), Qt::QueuedConnection);
        m_edit->appendPlainText(QString("Qt %1").arg(qVersion()));
        compressedConnect(&m_signaller, SIGNAL(emptySignal()), this, SLOT(emptySlot()));
        compressedConnect(&m_signaller, SIGNAL(dataSignal(int)), this, SLOT(dataSlot(int)));
    }
};

int main(int argc, char *argv[])
{
    CompressorApplication<QApplication> a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}

#include "main.moc"
