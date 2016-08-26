#include <QApplication>
#include <QMetaMethod>
#include <QMetaObject>
#include <private/qcoreapplication_p.h>
#include <private/qthread_p.h>
#include <private/qobject_p.h>
#include <algorithm>
#include <QDebug>

// Works on both Qt 4 and Qt 5.

/* Common Code */

/*! A connection.
 *  Note: The signal indices are as given by QMetaCallEvent.signalId.
 *  On Qt 5, those do *not* match QMetaObject::methodIndex since they
 *  exclude non-signal methods. */
class Connection {
public:
    enum { AnyIndex = -1 };
    //! Sender and receiver types; null is a wildcard for any type.
    const QMetaObject * from, * to;
    //! Sender and receiver instances; null is a wildcard for any instance.
    const QObject * sender, * receiver;
    //! Signal and slot indices. AnyIndex is a wildcard for any signal or slot.
    int signalIndex, slotIndex;
    Connection() : from(0), to(0), sender(0), receiver(0),
        signalIndex(AnyIndex), slotIndex(AnyIndex) {}
    Connection(Qt::Initialization) {}
    static Connection fromMetaCallEvent(const QMetaCallEvent * ev, const QObject * receiver) {
        Connection c(Qt::Uninitialized);
        c.from = ev->sender()->metaObject();
        c.to = receiver->metaObject();
        c.sender = ev->sender();
        c.receiver = receiver;
        c.signalIndex = ev->signalId();
        c.slotIndex = ev->id();
        if (c.slotIndex > 32767) c.slotIndex = AnyIndex;
        return c;
    }
    template <class T> static Connection fromSignal(const char * s) {
        Connection c;
        c.from = &T::staticMetaObject;
        c.signalIndex = getSignalIndex(c.from->method(c.from->indexOfSignal(s)));
        return c;
    }
    static Connection fromSignal(QObject * sender, const char * s) {
        Connection c;
        c.from = sender->metaObject();
        c.signalIndex = getSignalIndex(c.from->method(c.from->indexOfSignal(s)));
        return c;
    }
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    template <typename Signal> static Connection fromSignal(Signal s) {
        Connection c;
        QMetaMethod signal(QMetaMethod::fromSignal(s));
        c.from = signal.enclosingMetaObject();
        c.signalIndex = getSignalIndex(signal);
        return c;
    }
#endif
    bool like(const Connection & c2) {
        return from == c2.from && to == c2.to && sender == c2.sender
                && receiver == c2.receiver && signalIndex == c2.signalIndex
                && slotIndex == c2.slotIndex;
    }
    /*! Returns a signal index that can be compared to QMetaCallEvent.signalId. */
    static int getSignalIndex(const QMetaMethod & method) {
        Q_ASSERT(method.methodType() == QMetaMethod::Signal);
    #if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        int index = -1;
        const QMetaObject * mobj = method.enclosingMetaObject();
        for (int i = 0; i <= method.methodIndex(); ++i) {
            if (mobj->method(i).methodType() != QMetaMethod::Signal) continue;
            ++ index;
        }
        return index;
    #else
        return method.methodIndex();
    #endif
    }
};
QDebug operator<<(QDebug str, const Connection & c) {
    return str << "CompressedConnection:" << c.sender << c.from << c.signalIndex
               << c.receiver << c.to << c.slotIndex;
}

//! Treats both connections as patterns.
inline bool patternLessThan(const Connection & c1,
                            const Connection & c2) {
    bool equalFrom = !c1.from || !c2.from || c1.from == c2.from;
    if (! equalFrom) return c1.from < c2.from;
    bool equalTo = !c1.to || !c2.to || c1.to == c2.to;
    if (! equalTo) return c1.to < c2.to;
    bool equalSender = !c1.sender || !c2.sender || c1.sender == c2.sender;
    if (! equalSender) return c1.sender < c2.sender;
    bool equalReceiver = !c1.receiver || !c2.receiver || c1.receiver == c2.receiver;
    if (! equalReceiver) return c1.receiver < c2.receiver;
    bool equalSignalIndex = c1.signalIndex == Connection::AnyIndex
            || c2.signalIndex == Connection::AnyIndex
            || c1.signalIndex == c2.signalIndex;
    if (! equalSignalIndex) return c1.signalIndex < c2.signalIndex;
    bool equalSlotIndex = c1.slotIndex == Connection::AnyIndex
            || c2.slotIndex == Connection::AnyIndex
            || c1.slotIndex == c2.slotIndex;
    return equalSlotIndex ? false : c1.slotIndex < c2.slotIndex;
}

//! A pattern value won't match a non-pattern one.
inline bool strictLessThan(const Connection & c1, const Connection & c2) {
    return c1.from < c2.from || c1.to < c2.to || c1.sender < c2.sender
            || c1.receiver < c2.receiver || c1.signalIndex < c2.signalIndex
            || c1.slotIndex < c2.slotIndex;
}

//! Keeps a list of compressed connections.
class CompressedConnections {
    typedef QList<Connection> T;
    typedef std::pair<T::iterator, T::iterator> iter_pair;
    T m_data;
public:
    typedef T::const_iterator const_iterator;
    void clear() { m_data.clear(); }
    void add(const Connection & conn) {
        T::iterator it = std::lower_bound(m_data.begin(), m_data.end(), conn, strictLessThan);
        if (it != m_data.end() && it->like(conn)) *it = conn; else m_data.insert(it, conn);
    }
    void removeExact(const Connection & conn) {
        T::iterator it = std::lower_bound(m_data.begin(), m_data.end(), conn, strictLessThan);
        if (it != m_data.end()) m_data.erase(it);
    }
    void removeAll(const Connection & conn) {
        iter_pair range = std::equal_range(m_data.begin(), m_data.end(), conn, patternLessThan);
        m_data.erase(range.first, range.second);
    }
    bool containsMatching(const Connection & conn) const {
        return std::binary_search(m_data.begin(), m_data.end(), conn, patternLessThan);
    }
    const_iterator findFirst(const Connection & conn) const {
        return std::lower_bound(m_data.begin(), m_data.end(), conn, patternLessThan);
    }
    const_iterator end() const {
        return m_data.end();
    }
};

/* Implementation Using Event Compression With Access to Private Qt Headers */

struct EventHelper : private QEvent {
    static void clearPostedFlag(QEvent * ev) {
        (&static_cast<EventHelper*>(ev)->t)[1] &= ~0x8001;
    }
};

template <class Base> class CompressorApplication : public Base {
    CompressedConnections m_compressed;
public:
    CompressorApplication(int & argc, char ** argv) : Base(argc, argv) {}
    void addCompressedConnection(const Connection & conn)
    { m_compressed.add(conn); }
    void removeCompressedConnection(const Connection & conn)
    { m_compressed.removeExact(conn); }
    void removeAllCompressedConnections(const Connection & conn)
    { m_compressed.removeAll(conn); }
protected:
    bool compressEvent(QEvent *event, QObject *receiver,
                       QPostEventList *postedEvents) {
        if (event->type() != QEvent::MetaCall)
            return Base::compressEvent(event, receiver, postedEvents);

        QMetaCallEvent *mce = static_cast<QMetaCallEvent*>(event);
        if (! m_compressed.containsMatching(Connection::fromMetaCallEvent(mce, receiver))) return false;
        for (QPostEventList::iterator it = postedEvents->begin(); it != postedEvents->end(); ++it) {
            QPostEvent &cur = *it;
            if (cur.receiver != receiver || cur.event == 0 || cur.event->type() != event->type())
                continue;
            QMetaCallEvent *curMce = static_cast<QMetaCallEvent*>(cur.event);
            if (curMce->id() != mce->id()) continue;
            if (! m_compressed.containsMatching(Connection::fromMetaCallEvent(curMce, receiver))) continue;
            using std::swap;
            swap(cur.event, event); // Remove this line if you want to keep the oldest event instead
            // QEvent keeps track of whether it has been posted. Deletion of a formerly posted event
            // takes the posted event list mutex and does a useless search of the posted event
            // list upon deletion. We thus clear the QEvent::posted flag before deletion.
            EventHelper::clearPostedFlag(event);
            delete event;
            return true;
        }
        return false;
    }
};

//
// Demo GUI

#include <QWidget>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QFormLayout>

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
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        connect(invoke, &QPushButton::clicked, this, &Widget::sendSignals);
        connect(&m_signaller, &Signaller::emptySignal, this, &Widget::emptySlot, Qt::QueuedConnection);
        connect(&m_signaller, &Signaller::dataSignal, this, &Widget::dataSlot, Qt::QueuedConnection);
#else
        connect(invoke, SIGNAL(clicked()), SLOT(sendSignals()));
        connect(&m_signaller, SIGNAL(emptySignal()), SLOT(emptySlot()), Qt::QueuedConnection);
        connect(&m_signaller, SIGNAL(dataSignal(int)), SLOT(dataSlot(int)), Qt::QueuedConnection);
#endif
    }
};

int main(int argc, char *argv[])
{
    CompressorApplication<QApplication> a(argc, argv);
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    a.addCompressedConnection(Connection::fromSignal(&Signaller::emptySignal));
    a.addCompressedConnection(Connection::fromSignal(&Signaller::dataSignal));
#else
    a.addCompressedConnection(Connection::fromSignal<Signaller>("emptySignal()"));
    a.addCompressedConnection(Connection::fromSignal<Signaller>("dataSignal(int)"));
#endif
    Widget w;
    w.show();
    return a.exec();
}

#include "main.moc"
