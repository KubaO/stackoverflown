// https://github.com/KubaO/stackoverflown/tree/master/questions/proxy-reformat-39244309
#include <QtWidgets>

class RewriteProxy : public QIdentityProxyModel {
    QMap<QVariant, QVariant> m_read, m_write;
    int m_column;
public:
    RewriteProxy(int column, QObject * parent = nullptr) :
        QIdentityProxyModel{parent}, m_column{column} {}
    void addReadMapping(const QVariant & from, const QVariant & to) {
        m_read.insert(from, to);
        m_write.insert(to, from);
    }
    QVariant data(const QModelIndex & index, int role) const override {
        auto val = QIdentityProxyModel::data(index, role);
        if (index.column() != m_column) return val;
        auto it = m_read.find(val);
        return it != m_read.end() ? it.value() : val;
    }
    bool setData(const QModelIndex & index, const QVariant & value, int role) override {
        auto val = value;
        if (index.column() == m_column) {
            auto it = m_write.find(value);
            if (it != m_write.end()) val = it.value();
        }
        return QIdentityProxyModel::setData(index, val, role);
    }
};

int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    QStandardItemModel model{2,2};
    model.setData(model.index(0, 0), 1);
    model.setData(model.index(1, 0), 2);
    model.setData(model.index(0, 1), "Zaphod");
    model.setData(model.index(1, 1), "Beeblebrox");

    RewriteProxy proxy{0};
    proxy.setSourceModel(&model);
    proxy.addReadMapping(1, "Hello");
    proxy.addReadMapping(2, "World");

    QTableView ui;
    ui.setModel(&proxy);
    ui.show();
    return app.exec();
}
