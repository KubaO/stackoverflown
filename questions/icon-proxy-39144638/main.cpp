// https://github.com/KubaO/stackoverflown/tree/master/questions/icon-proxy-39144638
#include <QtWidgets>
#include <QtConcurrent>

/// A thread-safe function that returns an icon for an item with a given path.
/// If the icon is not known, a null icon is returned.
QIcon getIcon(const QString & path);

class IconProxy : public QIdentityProxyModel {
    Q_OBJECT
    QMap<QString, QIcon> m_icons;
    Q_SIGNAL void hasIcon(const QString&, const QIcon&, const QPersistentModelIndex& index) const;
    void onIcon(const QString& path, const QIcon& icon, const QPersistentModelIndex& index) {
        m_icons.insert(path, icon);
        emit dataChanged(index, index, QVector<int>{QFileSystemModel::FileIconRole});
    }
public:
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override {
        if (role == QFileSystemModel::FileIconRole) {
            auto path = index.data(QFileSystemModel::FilePathRole).toString();
            auto it = m_icons.find(path);
            if (it != m_icons.end()) {
                if (! it->isNull()) return *it;
                return QIdentityProxyModel::data(index, role);
            }
            QPersistentModelIndex pIndex{index};
            QtConcurrent::run([this,path,pIndex]{
                emit hasIcon(path, getIcon(path), pIndex);
            });
            return QVariant{};
        }
        return QIdentityProxyModel::data(index, role);
    }
    IconProxy(QObject * parent = nullptr) : QIdentityProxyModel{parent} {
        connect(this, &IconProxy::hasIcon, this, &IconProxy::onIcon);
    }
};

#include "main.moc"
