// https://github.com/KubaO/stackoverflown/tree/master/questions/metaproxy-32612309
#include <QtCore>
#include <QMetaProperty>
#include <functional>
#include <private/qmetaobjectbuilder_p.h>

class ObjectA : public QObject {
    Q_OBJECT
    Q_PROPERTY(int a MEMBER m_a NOTIFY aChanged)
    Q_PROPERTY(QString b READ b WRITE setB NOTIFY bChanged)
    int m_a;
    QString m_b;
public:
    ObjectA(QObject * parent = 0) : QObject(parent) {}
    QString b() const { return m_b; }
    Q_SLOT void setB(const QString & b) { if (m_b == b) return; emit bChanged(m_b = b); }
    Q_SLOT void slot(const QString & a, int c) {}
    Q_SIGNAL void aChanged(int a0);
    Q_SIGNAL void bChanged(const QString & b0);
};

class ObjectB : public ObjectA {
    Q_OBJECT
    Q_PROPERTY(QStringList c MEMBER m_c NOTIFY cChanged)
    QStringList m_c;
public:
    ObjectB(QObject * parent = 0) : ObjectA(parent) {}
    Q_SIGNAL void cChanged(const QStringList &);
    Q_SLOT void setC(const QStringList & c) { m_c = c; emit cChanged(c); }
};

struct CacheEntry {
    int typeId { 0 };
    void * value { nullptr };
    void setValue(void * src) {
        Q_ASSERT(value);
        qDebug() << "setting value" << QMetaType::typeName(typeId) << "@" << value;
        QMetaType::destruct(typeId, value);
        QMetaType::construct(typeId, value, src);
    }
    ~CacheEntry() {
        if (value) QMetaType::destroy(typeId, value);
    }
};
Q_DECLARE_TYPEINFO(CacheEntry, Q_MOVABLE_TYPE);

void dumpMethods(const QMetaObject * o) {
    qDebug() << o->className() << "has" << o->methodCount() << "methods";
    for (int i = 0; i < o->methodCount(); ++i) {
        qDebug() << "method" << i << o->method(i).name();
    }
}

/// Add all metaobjects in order from most base to most derived class,
/// with exception of QObject itself
void addMetaObjects(QMetaObjectBuilder & b, const QMetaObject * derived) {
    using Q = QMetaObjectBuilder;
    if (!derived->superClass()) return;
    addMetaObjects(b, derived->superClass());
    b.addMetaObject(derived, Q::AddMembers(Q::AllMembers) & ~Q::AddMembers(Q::SuperClass | Q::StaticMetacall));
}

class PropertyProxy : public QObject {
    QScopedPointer<QMetaObject, QScopedPointerPodDeleter> m_o;
    QPointer<QObject> m_target;
    QVector<CacheEntry> m_cache; // cache of property values
    typedef std::function<void(void**)> Slot;
    QVector<Slot> m_slots;

    int methodCount() const {
        return m_target->metaObject()->methodCount() + m_slots.size();
    }
    void readProperty(void * dst, int id) {
        void * args[1] = { dst };
        QObject * target = m_target;
        auto read = [&args, id, target]{
            target->qt_metacall(QMetaObject::ReadProperty, id, args);
        };
        if (target->thread() == QThread::currentThread()) {
            read();
        } else {
            QObject sig;
            connect(&sig, &QObject::destroyed, target, read, Qt::BlockingQueuedConnection);
        }
    }
public:
    Q_OBJECT_CHECK
    const QMetaObject *metaObject() const Q_DECL_OVERRIDE {
        return m_o.data();
    }
    int qt_metacall(QMetaObject::Call _c, int _id, void **_a) Q_DECL_OVERRIDE {
        qDebug() << __FUNCTION__ << _c << _id << _a;
        if (_c == QMetaObject::InvokeMetaMethod) {
            _id = m_target->QObject::qt_metacall(_c, _id, _a);
            if (_id < 0) return _id;
            if (_id < methodCount())
                qt_static_metacall(this, _c, _id, _a);
            _id -= methodCount();
            return _id;
        }
        return m_target->qt_metacall(_c, _id, _a);
    }
    static void qt_static_metacall(QObject *_o, QMetaObject::Call c, int id, void **a) {
        qDebug() << __FUNCTION__ << c << id << a;
        if (c != QMetaObject::InvokeMetaMethod) return;
        auto o = static_cast<PropertyProxy*>(_o);
        int targetMethodCount = o->m_target->metaObject()->methodCount() - QObject::staticMetaObject.methodCount();
        if (id < targetMethodCount) {
            id += QObject::staticMetaObject.methodCount();
            o->m_target->qt_metacall(c, id, a);
            return;
        }
        id -= targetMethodCount;
        Q_ASSERT(id >= 0 && id < o->m_slots.count());
        qDebug() << "calling slot" << id;
        o->m_slots.at(id)(a);
    }

    PropertyProxy(QObject * target, QObject * parent = 0) : QObject(parent), m_target(target) {
        typedef QMetaObjectBuilder Q;
        auto tmobj = target->metaObject();
        qDebug() << "target has" << tmobj->methodCount() << "methods";
        dumpMethods(tmobj);
        QMetaObjectBuilder b;
        b.setSuperClass(&QObject::staticMetaObject);
        b.setStaticMetacallFunction(qt_static_metacall);
        addMetaObjects(b, tmobj);
        int n = 0;
        for (int i = 0; i < tmobj->propertyCount(); ++i) {
            auto prop = tmobj->property(i);
            if (prop.isReadable() && prop.hasNotifySignal()) {
                m_cache.push_back(CacheEntry());
                m_slots.push_back(Slot());
                auto & entry = m_cache.last();
                entry.typeId = prop.userType();
                entry.value = QMetaType(prop.userType()).create();
                if (!entry.value) {
                    qWarning() << "PropertyProxy: cannot create an instance of the value of property" << prop.name()
                               << "of type" << QMetaType::typeName(prop.userType()) << "(" << prop.userType() << ")";
                }
                readProperty(entry.value, i);
                m_slots.last() = [this, n, prop](void ** args){
                    qDebug() << "notify slot" << n << "for" << prop.name();
                    m_cache[n].setValue(args[1]);
                };
                // create a slot that will be notified of the change of property value
                QByteArray s("void __pnslot__");
                s.append(prop.name());
                s.append("(");
                s.append(prop.typeName());
                s.append(")");
                QMetaMethodBuilder m = b.addSlot(s);
                int index = m.index() + QObject::metaObject()->methodCount();
                qDebug() << "adding notification slot" << s << "at index" << index;
                QMetaObject::connect(target, prop.notifySignalIndex(), this, index);
                n++;
            }
            else if (prop.isReadable()) {
                qWarning() << "PropertyProxy: skipping non-notifying property" << prop.name() << "of" << target;
            }
        }
        m_o.reset(b.toMetaObject());
        dumpMethods(m_o.data());
    }
};

int main(int argc, char ** argv) {
    QCoreApplication app{argc, argv};
    ObjectB b;
    PropertyProxy p(&b);
    b.setObjectName("yay");

}

#include "main.moc"
