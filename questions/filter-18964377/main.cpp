#include <QtGui>
#if QT_VERSION_MAJOR > 4
#include <QtWidgets>
#endif

class Vehicle {
   QString m_make, m_model, m_registrationNumber;
public:
   Vehicle(const QString & make, const QString & model, const QString & registrationNumber) :
      m_make{make}, m_model{model}, m_registrationNumber{registrationNumber} {}
   QString make() const { return m_make; }
   QString model() const { return m_model; }
   QString registrationNumber() const { return m_registrationNumber; }
   bool isRegistered() const { return !m_registrationNumber.isEmpty(); }
};

class VehicleModel : public QAbstractTableModel {
   QList<Vehicle> m_data;
public:
   VehicleModel(QObject * parent = nullptr) : QAbstractTableModel{parent} {}
   int rowCount(const QModelIndex &) const override { return m_data.count(); }
   int columnCount(const QModelIndex &) const override { return 3; }
   QVariant data(const QModelIndex &index, int role) const override {
      if (role != Qt::DisplayRole && role != Qt::EditRole) return QVariant{};
      const Vehicle & vehicle = m_data[index.row()];
      switch (index.column()) {
      case 0: return vehicle.make();
      case 1: return vehicle.model();
      case 2: return vehicle.registrationNumber();
      default: return QVariant{};
      };
   }
   QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
      if (orientation != Qt::Horizontal) return QVariant{};
      if (role != Qt::DisplayRole) return QVariant{};
      switch (section) {
      case 0: return "Make";
      case 1: return "Model";
      case 2: return "Reg.#";
      default: return QVariant{};
      }
   }
   void append(const Vehicle & vehicle) {
      beginInsertRows(QModelIndex{}, m_data.count(), m_data.count());
      m_data.append(vehicle);
      endInsertRows();
   }
};

class Widget : public QWidget {
   QGridLayout m_layout{this};
   QTableView m_view;
   QPushButton m_button{"Filter"};
   VehicleModel m_model;
   QSortFilterProxyModel m_proxy;
   QInputDialog m_dialog;
public:
   Widget() {
      m_layout.addWidget(&m_view, 0, 0, 1, 1);
      m_layout.addWidget(&m_button, 1, 0, 1, 1);
      connect(&m_button, SIGNAL(clicked()), &m_dialog, SLOT(open()));
      m_model.append(Vehicle{"Volvo", "240", "SQL8941"});
      m_model.append(Vehicle{"Volvo", "850", QString{}});
      m_model.append(Vehicle{"Volvo", "940", "QRZ1321"});
      m_model.append(Vehicle{"Volvo", "960", "QRZ1628"});
      m_proxy.setSourceModel(&m_model);
      m_proxy.setFilterKeyColumn(2);
      m_view.setModel(&m_proxy);
      m_dialog.setLabelText("Enter registration number fragment to filter on. Leave empty to clear filter.");
      m_dialog.setInputMode(QInputDialog::TextInput);
      connect(&m_dialog, SIGNAL(textValueSelected(QString)),
              &m_proxy, SLOT(setFilterFixedString(QString)));
   }
};

int main(int argc, char *argv[])
{
   QApplication a{argc, argv};
   Widget w;
   w.show();
   return a.exec();
}
