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
    int slotIndex { -1 };
    void setValue(void * src) {
        Q_ASSERT(value);
        qDebug() << "setting value" << QMetaType::typeName(typeId) << "@" << value;
        QMetaType::destruct(typeId, value);
        QMetaType::construct(typeId, value, src);
    }
    void getValue(void * dst) const {
        Q_ASSERT(value);
        qDebug() << "getting value" << QMetaType::typeName(typeId) << "@" << value;
        QMetaType::destruct(typeId, dst);
        QMetaType::construct(typeId, dst, value);
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


// from http://stackoverflow.com/a/24748637/1329652
int uint32_log2(uint32_t n)
{
#define S(k) if (n >= (UINT32_C(1) << k)) { i += k; n >>= k; }
    int i = -(n == 0); S(16); S(8); S(4); S(2); S(1); return i;
#undef S
}

int alignOf(int typeId) {
    size_t size = QMetaType::sizeOf(typeId);
    Q_ASSERT(size > 0);
    if (size > sizeof(void*)) size = sizeof(void*);
    return 1 << (uint32_log2(size)+1);
}

int align(int val, int alignment) {
    auto r = val % alignment;
    return r ? val + r : val;
}

int manageMember(int offset, int memberType, void * p = 0, int n = 0, void * copy = 0) {
    offset = align(offset, alignOf(memberType));
    if (p) {
        void * data = reinterpret_cast<char*>(p) + offset;
        if (copy) {
            reinterpret_cast<int*>(p)[n+1] = memberType;
            QMetaType::construct(memberType, data, copy);
        }
        else
            QMetaType::destruct(memberType, data);
    }
    return offset + QMetaType::sizeOf(memberType);
}

class Arguments {
    int types[1];
    Arguments(int n) {
        types[0] = n;
    }
    void free() {
        this->~Arguments();
        ::free(this);
    }

public:
    ~Arguments() {
        auto count = types[0];
        int offset = (count+1)*sizeof(int);
        for (int i = 0; i < count; ++i)
            offset = manageMember(offset, types[i+1], this, i);
    }
    static QSharedPointer<Arguments> make(const QMetaMethod & method, void ** args) {
        int N = method.parameterCount();
        auto iterate = [N, method](void * area, void ** args){
            int offset = (N+2)*sizeof(int);
            offset = manageMember(offset, method.returnType(), area, 0, args ? args[0] : nullptr);
            for (int i = 0; i < N; ++i) offset = manageMember(offset, method.parameterType(i), area, i+1, args ? args[i+1] : nullptr);
            return offset;
        };
        auto size = iterate(nullptr, nullptr);
        QSharedPointer<Arguments> p(reinterpret_cast<Arguments*>(malloc(size)), &Arguments::free);
        new (p.data()) Arguments(N+1);
        iterate(p.data(), args);
        return p;
    }
    void ** args() const { return nullptr; } // TODO
};

class ObjectProxy : public QObject {
    QScopedPointer<QMetaObject, QScopedPointerPodDeleter> m_o;
    QPointer<QObject> m_target;
    QVector<CacheEntry> m_cache; // cache of property values
    typedef std::function<void(void**)> Slot;
    QVector<Slot> m_slots;

    int methodCount() const {
        return m_target->metaObject()->methodCount() + m_slots.size();
    }
    void readProperty(void * dst, int id, bool warn = false) {
        void * args[1] = { dst };
        QObject * target = m_target;
        auto read = [&args, id, target]{
            target->qt_metacall(QMetaObject::ReadProperty, id, args);
        };
        if (target->thread() == QThread::currentThread()) {
            read();
        } else {
            if (warn) qWarning() << "blocking to read the property";
            QObject sig;
            connect(&sig, &QObject::destroyed, target, read, Qt::BlockingQueuedConnection);
        }
    }
    void writeProperty(void * src, int id) {
        QObject * target = m_target;
        if (target->thread() == QThread::currentThread()) {
            void * args[1] = { src };
            target->qt_metacall(QMetaObject::WriteProperty, id, args);
        } else {
            struct Data {
                int typeId;
                void * args[1] = { buf() };
                void * m_buf;
                void * buf() { return reinterpret_cast<void*>(&m_buf); }
                Data(int typeId, void * src) : typeId(typeId) {
                    QMetaType::construct(typeId, buf(), src);
                }
                ~Data() { QMetaType::destruct(typeId, buf()); }
                static Data * make(int typeId, void * src) {
                    Data * d = reinterpret_cast<Data*>(malloc(sizeof(Data) + QMetaType::sizeOf(typeId)));
                    return new (d) Data(typeId, src);
                }
                static void free(Data * d) {
                    d->~Data();
                    ::free(d);
                }
            };
            auto typeId = m_cache.at(id).typeId;
            QSharedPointer<Data> d(Data::make(typeId, src), Data::free);
            QObject sig;
            connect(&sig, &QObject::destroyed, target, [d, id, target]{
                target->qt_metacall(QMetaObject::WriteProperty, id, d->args);
            });
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
        else if (_c == QMetaObject::ReadProperty) {
            if (_id < m_o->propertyCount()) {
                auto prop = m_o->property(_id);
                if (prop.isReadable()) {
                    if (prop.hasNotifySignal()) {
                        m_cache.at(_id).getValue(_a[0]);
                    } else {
                        // TODO
                        // for cross-thread calls we should instead:
                        // 1. read the property in the target's thread
                        // 2. safely invoke our notification slot
                        // 3. return the cached value (even if wrong), taking the
                        //    read to mean "we'll need the value soon-ish"
                        // 4. all properties should then have synthetic notify signals, even
                        //    if they really don't
                        readProperty(_a[0], _id, true);
                    }
                }
            }
            _id -= m_o->propertyCount();
            return _id;
        }
        else if (_c == QMetaObject::WriteProperty) {
            if (_id < m_o->propertyCount()) {
                auto prop = m_o->property(_id);
                if (prop.isWritable())
                    writeProperty(_a[0], _id);
            }
            _id -= m_o->propertyCount();
            return _id;
        }
        return m_target->qt_metacall(_c, _id, _a);
    }
    static void qt_static_metacall(QObject *_o, QMetaObject::Call c, int id, void **a) {
        qDebug() << __FUNCTION__ << c << id << a;
        auto o = static_cast<ObjectProxy*>(_o);
        int targetMethodCount = o->m_target->metaObject()->methodCount() - QObject::staticMetaObject.methodCount();
        if (c == QMetaObject::InvokeMetaMethod) {
            if (id < targetMethodCount) {
                id += QObject::staticMetaObject.methodCount();
                if (o->m_target->thread() == QThread::currentThread())
                    o->m_target->qt_metacall(c, id, a);
                else {
                    // copy arguments for a cross-thread call
                    auto args = Arguments::make(o->m_target->metaObject()->method(id), a);
                    auto target = o->m_target;
                    QObject sig;
                    connect(&sig, &QObject::destroyed, o->m_target, [c, id, args, target]{
                        target->qt_metacall(c, id, args->args());
                    });
                }
                return;
            }
            id -= targetMethodCount;
            Q_ASSERT(id >= 0 && id < o->m_slots.count());
            qDebug() << "calling slot" << id;
            o->m_slots.at(id)(a);
        } else if (c == QMetaObject::IndexOfMethod) {
            if (id < targetMethodCount) {
                id += QObject::staticMetaObject.methodCount();
                o->m_target->qt_metacall(c, id, a);
            }
        }
    }

    ObjectProxy(QObject * target, QObject * parent = 0) : QObject(parent), m_target(target) {
        auto tmobj = target->metaObject();
        qDebug() << "target has" << tmobj->methodCount() << "methods";
        dumpMethods(tmobj);
        QMetaObjectBuilder b;
        b.setSuperClass(&QObject::staticMetaObject);
        b.setStaticMetacallFunction(qt_static_metacall);
        addMetaObjects(b, tmobj);
        int n = 0;
        m_cache.resize(tmobj->propertyCount());
        for (int i = 0; i < tmobj->propertyCount(); ++i) {
            auto prop = tmobj->property(i);
            if (prop.isReadable() && prop.hasNotifySignal()) {
                auto & entry = m_cache[i];
                entry.typeId = prop.userType();
                entry.value = QMetaType(prop.userType()).create();
                if (!entry.value) {
                    qWarning() << "PropertyProxy: cannot create an instance of the value of property" << prop.name()
                               << "of type" << QMetaType::typeName(prop.userType()) << "(" << prop.userType() << ")";
                }
                readProperty(entry.value, i);
                m_slots.push_back(Slot());
                m_slots.last() = [this, n, i, prop](void ** args){
                    qDebug() << "notify slot" << n << "for" << prop.name() << "index" << i;
                    m_cache[i].setValue(args[1]);
                };
                // create a slot that will be notified of the change of property value
                QByteArray s("void __pnslot__");
                s.append(prop.name());
                s.append("(");
                s.append(prop.typeName());
                s.append(")");
                QMetaMethodBuilder m = b.addSlot(s);
                int index = m.index() + QObject::metaObject()->methodCount();
                entry.slotIndex = index;
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
    ObjectProxy p(&b);
    b.setObjectName("yay");
    Q_ASSERT(b.objectName() == "yay");
    p.setProperty("objectName", "foo");
    Q_ASSERT(b.objectName() == "foo");
}

#include "main.moc"
