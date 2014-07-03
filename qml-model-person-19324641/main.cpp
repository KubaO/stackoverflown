#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QAbstractListModel>
#include <QQmlContext>
#include <QtQml>
#include <QSet>
#include <QBasicTimer>
#include <functional>

class Person : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name NOTIFY nameChanged MEMBER m_name)
    QString m_name;
public:
    Q_INVOKABLE Person(QObject * parent = 0) : QObject(parent) { setRandomName(); }
    Q_INVOKABLE Person(QString name, QObject * parent = 0) :
                       QObject(parent), m_name(name) {}
    Q_SIGNAL void nameChanged(const QString &);
    Q_INVOKABLE void setRandomName() {
        static const QString names = "Badger,Shopkeeper,Pepperpots,Gumbys,Colonel";
        static const QStringList nameList = names.split(',');
        QString newName = nameList.at(qrand() % nameList.length());
        if (newName != m_name) {
            m_name = newName;
            emit nameChanged(m_name);
        }
    }
};

class ObjectListModel : public QAbstractListModel {
    Q_OBJECT
    Q_DISABLE_COPY(ObjectListModel)
    //! Whether changes to underlying objects are exposed via `dataChanged` signals
    Q_PROPERTY(bool elementChangeTracking
               READ elementChangeTracking WRITE setElementChangeTracking
               NOTIFY elementChangeTrackingChanged)
    QObjectList m_data;
    std::function<QObject*()> m_factory;
    bool m_tracking;
    QBasicTimer m_notifyTimer;
    QMap<int, char> m_notifyIndexes;
    //! Updates the property tracking connections on given object.
    void updateTracking(QObject* obj) {
        const int nIndex = metaObject()->indexOfSlot("propertyNotification()");
        QMetaMethod const nSlot = metaObject()->method(nIndex);
        const int props = obj->metaObject()->propertyCount();
        if (m_tracking) for (int i = 0; i < props; ++i) {
            const QMetaProperty prop = obj->metaObject()->property(i);
            if (prop.hasNotifySignal()) connect(obj, prop.notifySignal(), this, nSlot);
        } else {
            disconnect(obj, 0, this, 0);
        }
    }
    //! Receives property notification changes
    Q_SLOT void propertyNotification() {
        int i = m_data.indexOf(sender());
        if (i >= 0) m_notifyIndexes.insert(i, 0);
        // All of the notifications will be sent as a single signal from the event loop.
        if (!m_notifyTimer.isActive()) m_notifyTimer.start(0, this);
    }
protected:
    //! Emits the notifications of changes done on the underlying QObject properties
    void timerEvent(QTimerEvent * ev) {
        if (ev->timerId() != m_notifyTimer.timerId()) return;
        emit dataChanged(index(m_notifyIndexes.begin().key()),
                         index((m_notifyIndexes.end()-1).key()),
                         QVector<int>(1, Qt::DisplayRole));
        m_notifyTimer.stop();
        m_notifyIndexes.clear();
    }
public:
    //! A model that creates instances via a given metaobject
    ObjectListModel(const QMetaObject * mo, QObject * parent = 0) :
        QAbstractListModel(parent),
        m_factory([mo, this](){
            return mo->newInstance(Q_ARG(QObject*, this));
        }),
        m_tracking(false)
    {}
    //! A model that creates instances using a factory function
    ObjectListModel(const std::function<QObject*()> & factory,
                    QObject * parent = 0) :
        QAbstractListModel(parent), m_factory(factory), m_tracking(false)
    {}
    ~ObjectListModel() {
        qDeleteAll(m_data);
    }
    bool elementChangeTracking() const { return m_tracking; }
    void setElementChangeTracking(bool tracking) {
        if (m_tracking == tracking) return;
        for (QObject* obj : m_data) updateTracking(obj);
        emit elementChangeTrackingChanged(m_tracking = tracking);
    }
    Q_SIGNAL void elementChangeTrackingChanged(bool);
    int rowCount(const QModelIndex &) const Q_DECL_OVERRIDE {
        return m_data.count();
    }
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE {
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return QVariant::fromValue(m_data.at(index.row()));
        }
        return QVariant();
    }
    bool setData(const QModelIndex &index, const QVariant &value, int role)
    Q_DECL_OVERRIDE {
        Q_UNUSED(role);
        QObject* object = value.value<QObject*>();
        if (!object) return false;
        if (object == m_data.at(index.row())) return true;
        delete m_data.at(index.row());
        m_data[index.row()] = object;
        emit dataChanged(index, index, QVector<int>(1, role));
        return true;
    }
    Q_INVOKABLE bool insertRows(int row, int count,
                                const QModelIndex &parent = QModelIndex())
    Q_DECL_OVERRIDE {
        Q_UNUSED(parent);
        beginInsertRows(QModelIndex(), row, row + count - 1);
        for (int i = row; i < row + count; ++ i) {
            QObject * object = m_factory();
            Q_ASSERT(object);
            m_data.insert(i, object);
            updateTracking(object);
            QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);
        }
        endInsertRows();
        return true;
    }
    Q_INVOKABLE bool removeRows(int row, int count,
                                const QModelIndex &parent = QModelIndex())
    Q_DECL_OVERRIDE {
        Q_UNUSED(parent);
        beginRemoveRows(QModelIndex(), row, row + count - 1);
        while (count--) delete m_data.takeAt(row);
        endRemoveRows();
        return true;
    }
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    qmlRegisterType<Person>();
    ObjectListModel model1(&Person::staticMetaObject);
    model1.setElementChangeTracking(true);
    model1.insertRows(0, 1);
    engine.rootContext()->setContextProperty("model1", &model1);
    engine.load(QUrl("qrc:/main.qml"));
    QObject *topLevel = engine.rootObjects().value(0);
    QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel);
    window->show();
    return app.exec();
}

#include "main.moc"
