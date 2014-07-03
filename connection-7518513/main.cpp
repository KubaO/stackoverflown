#include <QCoreApplication>
#include <QMetaMethod>
#include <QPointer>

class Connection : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged USER true)
    Q_PROPERTY(bool valid READ isValid)
    QMetaMethod m_signal, m_slot;
    QPointer<QObject> m_source, m_target;
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QMetaObject::Connection m_connection;
#else
    bool m_connection;
#endif
    bool m_active;
    void release() {
        if (!m_source || !m_target) return;
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        disconnect(m_connection);
#else
        disconnect(m_source, m_signal, m_target, m_slot);
#endif
    }
public:
    Connection(QObject * source, const char * signal, QObject * target, const char * slot, QObject * parent = 0) :
        QObject(parent),
        m_signal(source->metaObject()->method(source->metaObject()->indexOfSignal(signal))),
        m_slot(target->metaObject()->method(target->metaObject()->indexOfSlot(slot))),
        m_source(source), m_target(target),
        m_connection(connect(m_source, m_signal, m_target, m_slot)),
        m_active(m_connection)
    {}
    ~Connection() { release(); }
    QObject* source() const { return m_source; }
    QObject* target() const { return m_target; }
    QMetaMethod signal() const { return m_signal; }
    QMetaMethod slot() const { return m_slot; }
    bool isActive() const { return m_active && m_source && m_target; }
    bool isValid() const { return m_connection && m_source && m_target; }
    Q_SIGNAL void activeChanged(bool);
    Q_SLOT void setActive(bool active) {
        if (active == m_active || !m_source || !m_target) return;
        m_active = active;
        if (m_active) {
            m_connection = connect(m_source, m_signal, m_target, m_slot);
        } else {
            release();
        }
        emit activeChanged(m_active);
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    return a.exec();
}

#include "main.moc"
