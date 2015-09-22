// https://github.com/KubaO/stackoverflown/tree/master/questions/metaproxy-32612309
#include <QtCore>
#include <QMetaProperty>
#include <functional>

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

static const uint qt_meta_data_PropertyProxy[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

struct qt_meta_stringdata_PropertyProxy_t {
    QByteArrayData data[1];
    char stringdata[14];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PropertyProxy_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PropertyProxy_t qt_meta_stringdata_PropertyProxy = {
    {
QT_MOC_LITERAL(0, 0, 13), // "PropertyProxy"
    },
    "PropertyProxy"
};
#undef QT_MOC_LITERAL

class ByteArrayData {
    QByteArrayData data;
public:
    ByteArrayData() {}
    ByteArrayData(const QByteArray & other) :
        data { Q_REFCOUNT_INITIALIZE_STATIC, other.size(), 0, 0, other.constData() - reinterpret_cast<char*>(this) }
    {}
    ByteArrayData(const ByteArrayData & other) {
        *this = other;
    }
    ByteArrayData & operator=(const ByteArrayData & other) {
        memcpy(&data, &other.data, sizeof(data));
        data.offset -= (reinterpret_cast<char*>(&data)-reinterpret_cast<const char*>(&other.data));
        return *this;
    }
    QByteArrayDataPtr ptr() {
        QByteArrayDataPtr p { &data };
        return p;
    }
};

class PropertyProxy : public QObject {
#if 0
public: \
    Q_OBJECT_CHECK \
    static const QMetaObject staticMetaObject; \
    virtual const QMetaObject *metaObject() const; \
    virtual void *qt_metacast(const char *); \
    QT_TR_FUNCTIONS \
    virtual int qt_metacall(QMetaObject::Call, int, void **); \
private: \
    Q_DECL_HIDDEN_STATIC_METACALL static void qt_static_metacall(QObject *, QMetaObject::Call, int, void **); \
    struct QPrivateSignal {};
#endif
    QObject * m_target { nullptr };
    QMetaObject m_metaProxy;
    QVector<ByteArrayData> m_stringData;
    QList<QByteArray> m_strings; // do not modify the arrays stored here!

    int addString(const QByteArray & str) {
        auto it = std::find(m_strings.cbegin(), m_strings.cend(), str);
        if (it != m_strings.cend())
            return it - m_strings.cbegin();
        m_strings.append(str);
        m_strings.last().detach();
        m_stringData.append(ByteArrayData(m_strings.last()));
        return m_strings.size() - 1;
    }

    struct CacheEntry {
        int typeId;
        void * value { nullptr };
        void setValue(void * src) {
            Q_ASSERT(value);
            QMetaType::destruct(typeId, value);
            QMetaType::construct(typeId, value, src);
        }
        ~CacheEntry() {
            if (value) QMetaType::destroy(typeId, value);
        }
    };
    QVector<CacheEntry> m_cache; // cache of property values

    typedef std::function<void(void**)> Slot;
    QVector<Slot> m_slots;

private:
    Q_DECL_HIDDEN_STATIC_METACALL static void qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
    {
        Q_UNUSED(_o);
        Q_UNUSED(_id);
        Q_UNUSED(_c);
        Q_UNUSED(_a);
    }
    void readProperty(void * dst, QObject * target, int id) {
        void * args[1] = { dst };
        auto read = [&args, id, target]{
            target->qt_metacall(QMetaObject::ReadProperty, id, args);
        };
        if (target->thread() == thread()) {
            read();
        } else {
            QObject sig;
            connect(&sig, &QObject::destroyed, target, read, Qt::BlockingQueuedConnection);
        }
    }
    /// Add a dynamic slot that invokes a given functor
    int addSlot(std::function<void(void**)> && fun) {
        m_slots.push_back(std::move(fun));
        return QObject::staticMetaObject.methodCount() + m_slots.count() - 1;
    }
public:
    Q_OBJECT_CHECK
    PropertyProxy(QObject * parent = 0) : QObject(parent) {
        qDebug() << addString("abcd");
        qDebug() << "d_array" << hex << (void*)m_stringData.data();
        qDebug() << "d_array data" << hex << (void*)m_stringData[0].ptr().ptr->data();
        qDebug() << "d_ptr data" << hex << (void*)m_strings[0].constData();
    }
    PropertyProxy(QObject * target, QObject * parent = 0) : QObject(parent), m_target(target) {
        auto mobj = target->metaObject();
        m_cache.resize(mobj->propertyCount());
        for (int i = 0; i < m_cache.size(); ++i) {
            auto prop = mobj->property(i);
            if (prop.isReadable() && prop.hasNotifySignal()) {
                // cache the property
                m_cache[i].typeId = prop.userType();
                m_cache[i].value = QMetaType(prop.userType()).create();
                if (!m_cache[i].value) {
                    qWarning() << "PropertyProxy: cannot create an instance of the value of property" << prop.name()
                               << "of type" << QMetaType::typeName(prop.userType()) << "(" << prop.userType() << ")";
                }
                readProperty(m_cache[i].value, target, i);
                // create a slot that will be notified of the change of property value
                int slot = addSlot([this, i](void ** args){ m_cache[i].setValue(args[1]); });
                QMetaObject::connect(target, prop.notifySignalIndex(), this, slot);
            }
            else if (prop.isReadable()) {
                qWarning() << "PropertyProxy: skipping non-notifying property" << prop.name() << "of" << target;
            }
        }


        m_metaProxy.d.superdata = &QObject::staticMetaObject;

    }


    static QMetaObject staticMetaObject;
    const QMetaObject *metaObject() const Q_DECL_OVERRIDE
    {
        return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
    }
    void *qt_metacast(const char *_clname) Q_DECL_OVERRIDE
    {
        if (!_clname) return Q_NULLPTR;
        if (!strcmp(_clname, qt_meta_stringdata_PropertyProxy.stringdata))
            return static_cast<void*>(const_cast< PropertyProxy*>(this));
        return QObject::qt_metacast(_clname);
    }
    int qt_metacall(QMetaObject::Call _c, int _id, void **_a) Q_DECL_OVERRIDE
    {
        _id = QObject::qt_metacall(_c, _id, _a);
        if (_id < 0)
            return _id;

        return _id;
    }


};

QMetaObject PropertyProxy::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_PropertyProxy.data,
      qt_meta_data_PropertyProxy,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


int main(int argc, char ** argv) {
    QCoreApplication app{argc, argv};
    PropertyProxy p;
}

//

#if 0

struct qt_meta_stringdata_Object_t {
    QByteArrayData data[6];
    char stringdata[30];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Object_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Object_t qt_meta_stringdata_Object = {
    {
QT_MOC_LITERAL(0, 0, 6), // "Object"
QT_MOC_LITERAL(1, 7, 8), // "aChanged"
QT_MOC_LITERAL(2, 16, 0), // ""
QT_MOC_LITERAL(3, 17, 1), // "a"
QT_MOC_LITERAL(4, 19, 8), // "bChanged"
QT_MOC_LITERAL(5, 28, 1) // "b"

    },
    "Object\0aChanged\0\0a\0bChanged\0b"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Object[] = {

 // content:
       7,       // revision  0
       0,       // classname 1
       0,    0, // classinfo  count offset 2
       2,   14, // methods    count offset 4
       2,   30, // properties count offset 6
       0,    0, // enums/sets 8
       0,    0, // constructors 10
       0,       // flags  12
       2,       // signalCount 13

 // signals: name, argc, parameters offset, tag, flags
       1,    1,   24,    2, 0x06 /* Public 14 */,
       4,    1,   27,    2, 0x06 /* Public 19 */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,      // 24
    QMetaType::Void, QMetaType::QString,    5,  // 27

 // properties: name, type, flags
       3, QMetaType::Int, 0x00495003,          // 30
       5, QMetaType::QString, 0x00495103,      // 33

 // properties: notify_signal_id
       0,       // 34
       1,       // 35

       0        // eod
};

void Object::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Object *_t = static_cast<Object *>(_o);
        switch (_id) {
        case 0: _t->aChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->bChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (Object::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Object::aChanged)) {
                *result = 0;
            }
        }
        {
            typedef void (Object::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Object::bChanged)) {
                *result = 1;
            }
        }
    }
}

const QMetaObject Object::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Object.data,
      qt_meta_data_Object,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *Object::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Object::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_Object.stringdata))
        return static_cast<void*>(const_cast< Object*>(this));
    return QObject::qt_metacast(_clname);
}

int Object::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = m_a; break;
        case 1: *reinterpret_cast< QString*>(_v) = b(); break;
        default: break;
        }
        _id -= 2;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0:
            if (m_a != *reinterpret_cast< int*>(_v)) {
                m_a = *reinterpret_cast< int*>(_v);
                Q_EMIT aChanged(m_a);
            }
            break;
        case 1: setB(*reinterpret_cast< QString*>(_v)); break;
        default: break;
        }
        _id -= 2;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 2;
    } else if (_c == QMetaObject::RegisterPropertyMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void Object::aChanged(int _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Object::bChanged(const QString & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

#endif

#include "main.moc"
