#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QFormLayout>
#include <QSet>
#include <QMetaMethod>
#include <QDebug>

// Common Code

/*! Keeps a list of method indices for one or more meatobject classes. */
class MethodList {
    Q_DISABLE_COPY(MethodList)
    typedef QMap<const QMetaObject *, QSet<int> > T;
    T m_data;
public:
    MethodList() {}
    template <class T> void add(const char * slot) {
        add(T::staticMetaObject.method(T::staticMetaObject.indexOfSlot(slot)));
    }
    void add(const QMetaMethod & method) {
        Q_ASSERT(method.methodIndex() >= 0);
        m_data[method.enclosingMetaObject()].insert(method.methodIndex());
    }
    void remove(const QMetaMethod & method) {
        T::iterator it = m_data.find(method.enclosingMetaObject());
        if (it != m_data.end()) {
            it->remove(method.methodIndex());
            if (it->empty()) m_data.erase(it);
        }
    }
    bool contains(const QMetaObject * metaObject, int methodId) {
        T::const_iterator it = m_data.find(metaObject);
        return it != m_data.end() && it.value().contains(methodId);
    }
};
Q_GLOBAL_STATIC(MethodList, compressedSlots)

// Compressor

class Compressor : public QObject {
    enum { Idle, Armed, Valid } m_state;
    QMetaObject::Call m_call;
    int m_methodIndex;
    QSet<int> m_armed; // armed method IDs

    int qt_metacall(QMetaObject::Call call, int id, void ** args) {
        if (m_state != Armed) return QObject::qt_metacall(call, id, args);
        m_state = Valid;
        m_call = call;
        m_methodIndex = id;
        return 0;
    }
    bool eventFilter(QObject * target, QEvent * ev) {
        Q_ASSERT(target == parent());
        if (ev->type() == QEvent::MetaCall) {
            m_state = Armed;
            if (QT_VERSION < QT_VERSION_CHECK(5,0,0) || ! *(void**)(ev+1)) {
                // On Qt5, we ensure null QMetaCallEvent::slotObj_ since we can't handle Qt5-style member pointer calls
                // Note: This will dispatch QObject methods to this class. This is a bug without a workaround!
                Compressor::event(ev); // Use QObject::event() and qt_metacall to extract metacall data
            }
            if (m_state == Armed) m_state = Idle;
            // Only intercept compressed slot calls
            if (m_state != Valid || m_call != QMetaObject::InvokeMetaMethod ||
                    ! compressedSlots()->contains(target->metaObject(), m_methodIndex)) return false;
            int methodIndex = m_methodIndex;
            m_armed.insert(methodIndex);
            QCoreApplication::sendPostedEvents(target, QEvent::MetaCall); // recurse
            if (! m_armed.contains(methodIndex)) return true; // Compress the call
            m_armed.remove(methodIndex);
        }
        return false;
    }
public:
    Compressor(QObject * parent) : QObject(parent), m_state(Idle) {
        qDebug() << d_ptr->metaObject;
        parent->installEventFilter(this);
    }
};

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
    QPlainTextEdit * m_edit;
    QSpinBox * m_count;
    Signaller m_signaller;
    Q_SLOT void emptySlot() {
        m_edit->appendPlainText("emptySlot invoked");
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
        connect(invoke, SIGNAL(clicked()), SLOT(sendSignals()));
        m_edit->appendPlainText(QString("Qt %1").arg(qVersion()));
        connect(&m_signaller, SIGNAL(emptySignal()), SLOT(emptySlot()), Qt::QueuedConnection);
        connect(&m_signaller, SIGNAL(dataSignal(int)), this, SLOT(dataSlot(int)), Qt::QueuedConnection);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    compressedSlots()->add<Widget>("emptySlot()");
    compressedSlots()->add<Widget>("dataSlot(int)");
    Widget w;
    new Compressor(&w);
    w.show();
    return a.exec();
}

#include "main.moc"

#if 0

//! Extracts metacall details from a QMetaCallEvent without using its definition.
//! Works for Qt4-style slot calls, not for Qt5-style member pointer calls.
class MetaCall {
    class Helper : public QObject {
        MetaCall * m_call;
        int qt_metacall(QMetaObject::Call call, int id, void ** args) {
            if (! m_call) return QObject::qt_metacall(call, id, args);
            m_call->m_call = call;
            m_call->m_methodIndex = id;
            return 0;
        }
    public:
        Helper() : m_call(0) {}
        void extract(MetaCall * call, QEvent * event) {
            Q_ASSERT(QThread::currentThread() == thread());
            m_call = call;
            if (QT_VERSION < QT_VERSION_CHECK(5,0,0) || ! *(void**)(event+1)) {
                // On Qt5, we ensure null QMetaCallEvent::slotObj_ since we can't handle Qt5-style member pointer calls
                QObject::event(event); // Calls into qt_metacall
            }
            m_call = 0;
        }
    };
    int m_methodIndex;
    QMetaObject::Call m_call;
    static QThreadStorage<Helper> m_helper;
public:
    explicit MetaCall(QEvent * event) : m_methodIndex(-1) {
        if (event->type() == QEvent::MetaCall) m_helper.localData().extract(this, event);
    }
    bool isValid() const { return m_methodIndex >= 0; }
    int methodIndex() const { return m_methodIndex; }
    QMetaObject::Call callType() const { return m_call; }
};

#endif
