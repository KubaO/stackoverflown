#include <QApplication>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QInputDialog>
#include <QTableView>
#include <QGridLayout>
#include <QPushButton>
#include <QDebug>

class Vehicle {
    QString m_make, m_model, m_registrationNumber;
public:
    Vehicle(const QString & make, const QString & model, const QString & registrationNumber = QString()) :
        m_make(make), m_model(model), m_registrationNumber(registrationNumber) {}
    QString make() const { return m_make; }
    QString model() const { return m_model; }
    QString registrationNumber() const { return m_registrationNumber; }
    bool isRegistered() const { return !m_registrationNumber.isEmpty(); }
};

class VehicleModel : public QAbstractTableModel {
    QList<Vehicle> m_data;
public:
    VehicleModel(QObject * parent = 0) : QAbstractTableModel(parent) {}
    int rowCount(const QModelIndex &) const { return m_data.count(); }
    int columnCount(const QModelIndex &) const { return 3; }
    QVariant data(const QModelIndex &index, int role) const {
        if (role != Qt::DisplayRole && role != Qt::EditRole) return QVariant();
        const Vehicle & vehicle = m_data[index.row()];
        switch (index.column()) {
        case 0: return vehicle.make();
        case 1: return vehicle.model();
        case 2: return vehicle.registrationNumber();
        default: return QVariant();
        };
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const {
        if (orientation != Qt::Horizontal) return QVariant();
        if (role != Qt::DisplayRole) return QVariant();
        switch (section) {
        case 0: return "Make";
        case 1: return "Model";
        case 2: return "Reg.#";
        default: return QVariant();
        }
    }
    void append(const Vehicle & vehicle) {
        beginInsertRows(QModelIndex(), m_data.count(), m_data.count());
        m_data.append(vehicle);
        endInsertRows();
    }
};

class Widget : public QWidget {
    Q_OBJECT
    VehicleModel m_model;
    QSortFilterProxyModel m_proxy;
    Q_SLOT void setFilter() {
        QInputDialog * dialog = new QInputDialog(this);
        dialog->setLabelText("Enter registration number fragment to filter on. Leave empty to clear filter.");
        dialog->setInputMode(QInputDialog::TextInput);
        dialog->open(&m_proxy, SLOT(setFilterFixedString(QString)));
        dialog->setAttribute(Qt::WA_DeleteOnClose);
    }
public:
    Widget() {
        QGridLayout * layout = new QGridLayout(this);
        QTableView * view = new QTableView;
        layout->addWidget(view, 0, 0, 1, 1);
        QPushButton * btn = new QPushButton("Filter");
        layout->addWidget(btn, 1, 0, 1, 1);
        connect(btn, SIGNAL(clicked()), SLOT(setFilter()));
        m_model.append(Vehicle("Volvo", "240", "SQL8941"));
        m_model.append(Vehicle("Volvo", "850"));
        m_model.append(Vehicle("Volvo", "940", "QRZ1321"));
        m_model.append(Vehicle("Volvo", "960", "QRZ1628"));
        m_proxy.setSourceModel(&m_model);
        m_proxy.setFilterKeyColumn(2);
        view->setModel(&m_proxy);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}

#include "main.moc"
