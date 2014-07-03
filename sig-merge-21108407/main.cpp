#include <QCoreApplication>
#include <QSet>
#include <QMetaObject>

class SignalMerge : public QObject {
    Q_OBJECT
#if QT_VERSION>=QT_VERSION_CHECK(5,0,0)
    typedef QMetaObject::Connection Connection;
#else
    typedef bool Connection;
#endif
    typedef QPair<QObject*, int> ObjectMethod;
    QSet<ObjectMethod> m_signals, m_pendingSignals;

    void registerSignal(QObject * obj, const char * method) {
        int index = obj->metaObject()->indexOfMethod(method);
        if (index < 0) return;
        m_signals.insert(ObjectMethod(obj, index));
    }
    Q_SLOT void merge() {
        if (m_pendingSignals.isEmpty()) m_pendingSignals = m_signals;
        m_pendingSignals.remove(ObjectMethod(sender(), senderSignalIndex()));
        if (m_pendingSignals.isEmpty()) emit merged();
    }
public:
    void clear() {
        foreach (ObjectMethod om, m_signals) {
            QMetaObject::disconnect(om.first, om.second, this, staticMetaObject.indexOfSlot("merge()"));
        }
        m_signals.clear();
        m_pendingSignals.clear();
    }
    Q_SIGNAL void merged();
    Connection connect(QObject *sender, const char *signal, Qt::ConnectionType type = Qt::AutoConnection) {
        Connection conn = QObject::connect(sender, signal, this, SLOT(merge()), type);
        if (conn) registerSignal(sender, QMetaObject::normalizedSignature(signal+1));
        return conn;
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    return a.exec();
}

#include "main.moc"
