// https://github.com/KubaO/stackoverflown/tree/master/questions/proxy-combo-31995345
#include <QtWidgets>
#include <QtSql>

const QString kYes = QStringLiteral("Yes");
const QString kNo = QStringLiteral("No");

/// Maps a given column from {0,1} to {false, true} for editing and {kNo, kYes} for display
class BoolProxy : public QIdentityProxyModel {
    int m_column;
public:
    BoolProxy(int column, QObject * parent = nullptr) :
        QIdentityProxyModel{parent}, m_column{column} {}
    QVariant data(const QModelIndex & index, int role) const override {
        auto val = QIdentityProxyModel::data(index, role);
        if (index.column() != m_column) return val;
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            if (val.toInt() == 0) {
                if (role == Qt::DisplayRole) return kNo;
                return false;
            } else {
                if (role == Qt::DisplayRole) return kYes;
                return true;
            }
        }
        return val;
    }
    bool setData(const QModelIndex & index, const QVariant & value, int role) override {
        auto val = value;
        if (index.column() == m_column && role == Qt::EditRole)
            val = val.toBool() ? 1 : 0;
        return QIdentityProxyModel::setData(index, val, role);
    }
};

class YesNoDelegate : public QStyledItemDelegate {
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override {
        auto ed = QStyledItemDelegate::createEditor(parent, option, index);
        auto combo = qobject_cast<QComboBox*>(ed);
        combo->setItemText(0, kNo);
        combo->setItemText(1, kYes);
        return ed;
    }
};

int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (! db.open()) return 1;
    db.exec("create table example (name text, inuse numeric);");
    db.exec("insert into example (name, inuse) values "
            "('Zaphod', 0), ('Beeblebrox', 1);");

    QSqlTableModel model;
    model.setTable("example");
    model.select();

    BoolProxy proxy{1};
    proxy.setSourceModel(&model);

    QTableView ui;
    YesNoDelegate delegate;
    ui.setItemDelegateForColumn(1, &delegate);
    ui.setModel(&proxy);
    ui.show();
    return app.exec();
}
