#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QAbstractListModel>
#include <QQmlContext>
#include <QtQml>

class GPage : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name NOTIFY nameChanged MEMBER m_name)
    Q_PROPERTY(int number NOTIFY numberChanged MEMBER m_number)
    QString m_name;
    int m_number;
public:
    GPage(QObject * parent = 0) : QObject(parent), m_number(0) {}
    GPage(QString name, int number, QObject * parent = 0) :
        QObject(parent), m_name(name), m_number(number) {}
    Q_SIGNAL void nameChanged(const QString &);
    Q_SIGNAL void numberChanged(int);
};

class PageModel : public QAbstractListModel {
    Q_OBJECT
    QList<GPage*> m_pageList;
public:
    PageModel(QObject * parent = 0) : QAbstractListModel(parent) {}
    ~PageModel() { qDeleteAll(m_pageList); }
    int rowCount(const QModelIndex &) const Q_DECL_OVERRIDE {
        return m_pageList.count();
    }
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE {
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return QVariant::fromValue<QObject*>(m_pageList.at(index.row()));
        }
        return QVariant();
    }
    bool setData(const QModelIndex &index, const QVariant &value, int role) Q_DECL_OVERRIDE {
        Q_UNUSED(role);
        GPage* page = value.value<GPage*>();
        if (!page) return false;
        if (page == m_pageList.at(index.row())) return true;
        delete m_pageList.at(index.row());
        m_pageList[index.row()] = page;
        QVector<int> roles;
        roles << role;
        emit dataChanged(index, index, roles);
        return true;
    }
    Q_INVOKABLE bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE {
        Q_UNUSED(parent);
        beginInsertRows(QModelIndex(), row, row + count - 1);
        for (int i = row; i < row + count; ++ i) {
            QString const name = QString("Page %1").arg(i + 1);
            GPage * page = new GPage(name, i + 1, this);
            m_pageList.insert(i, page);
            QQmlEngine::setObjectOwnership(page, QQmlEngine::CppOwnership);
        }
        endInsertRows();
        return true;
    }
    Q_INVOKABLE bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE {
        Q_UNUSED(parent);
        beginRemoveRows(QModelIndex(), row, row + count - 1);
        while (count--) delete m_pageList.takeAt(row);
        endRemoveRows();
        return true;
    }
};

int main(int argc, char *argv[])
{
    PageModel model1;
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    model1.insertRows(0, 1);
    engine.rootContext()->setContextProperty("model1", &model1);
    qmlRegisterType<GPage>();
    engine.load(QUrl("qrc:/main.qml"));
    QObject *topLevel = engine.rootObjects().value(0);
    QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel);
    window->show();
    return app.exec();
}

#include "main.moc"
