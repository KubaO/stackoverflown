#include <QApplication>
#include <QTableView>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QIdentityProxyModel>
#include <QSqlDatabase>
#include <QMap>
#include <QVBoxLayout>
#include <QPushButton>

// Lexicographic comparison for a variant list
bool operator<(const QVariantList &a, const QVariantList &b) {
   int count = std::max(a.count(), b.count());
   // For lexicographic comparison, null comes before all else
   Q_ASSERT(QVariant() < QVariant::fromValue(-1));
   for (int i = 0; i < count; ++i) {
      auto aValue = i < a.count() ? a.value(i) : QVariant();
      auto bValue = i < b.count() ? b.value(i) : QVariant();
      if (aValue < bValue) return true;
   }
   return false;
}

class RowSelectionEmulatorProxy : public QIdentityProxyModel {
   Q_OBJECT
   Q_PROPERTY(QBrush selectedBrush READ selectedBrush WRITE setSelectedBrush)
   QMap<QVariantList, QModelIndex> mutable m_selection;
   QVector<int> m_roles;
   QBrush m_selectedBrush;
   bool m_ignoreReset;
   class SqlTableModel : public QSqlTableModel {
   public:
      using QSqlTableModel::primaryValues;
   };
   SqlTableModel * source() const {
      return static_cast<SqlTableModel*>(dynamic_cast<QSqlTableModel*>(sourceModel()));
   }
   QVariantList primaryValues(int row) const {
      auto record = source()->primaryValues(row);
      QVariantList values;
      for (int i = 0; i < record.count(); ++i) values << record.field(i).value();
      return values;
   }
   void notifyOfChanges(int row) {
      emit dataChanged(index(row, 0), index(row, columnCount()-1), m_roles);
   }
   void notifyOfAllChanges(bool remove = false) {
      auto it = m_selection.begin();
      while (it != m_selection.end()) {
         if (it->isValid()) notifyOfChanges(it->row());
         if (remove) it = m_selection.erase(it); else ++it;
      }
   }
public:
   RowSelectionEmulatorProxy(QObject* parent = 0) :
      QIdentityProxyModel(parent), m_roles(QVector<int>() << Qt::BackgroundRole),
      m_ignoreReset(false) {
      connect(this, &QAbstractItemModel::modelReset, [this]{
         if (! m_ignoreReset) {
            m_selection.clear();
         } else {
            for (auto it = m_selection.begin(); it != m_selection.end(); ++it) {
               *it = QModelIndex(); // invalidate the cached mapping
            }
         }
      });
   }
   QBrush selectedBrush() const { return m_selectedBrush; }
   void setSelectedBrush(const QBrush & brush) {
      if (brush == m_selectedBrush) return;
      m_selectedBrush = brush;
      notifyOfAllChanges();
   }
   QList<int> selectedRows() const {
      QList<int> result;
      for (auto it = m_selection.begin(); it != m_selection.end(); ++it) {
         if (it->isValid()) result << it->row();
      }
      return result;
   }
   bool isRowSelected(const QModelIndex &proxyIndex) const {
      if (! source() || proxyIndex.row() >= rowCount()) return false;
      auto primaryKey = primaryValues(proxyIndex.row());
      return m_selection.contains(primaryKey);
   }
   Q_SLOT void selectRow(const QModelIndex &proxyIndex, bool selected = true) {
      if (! source() || proxyIndex.row() >= rowCount()) return;
      auto primaryKey = primaryValues(proxyIndex.row());
      if (selected) {
         m_selection.insert(primaryKey, proxyIndex);
      } else {
         m_selection.remove(primaryKey);
      }
      notifyOfChanges(proxyIndex.row());
   }
   Q_SLOT void toggleRowSelection(const QModelIndex &proxyIndex) {
      selectRow(proxyIndex, !isRowSelected(proxyIndex));
   }
   Q_SLOT virtual void clearSelection() {
      notifyOfAllChanges(true);
   }
   QVariant data(const QModelIndex &proxyIndex, int role) const Q_DECL_OVERRIDE {
      QVariant value = QIdentityProxyModel::data(proxyIndex, role);
      if (proxyIndex.row() < rowCount() && source()) {
         auto primaryKey = primaryValues(proxyIndex.row());
         auto it = m_selection.find(primaryKey);
         if (it != m_selection.end()) {
            // update the cache
            if (! it->isValid()) *it = proxyIndex;
            // return the background
            if (role == Qt::BackgroundRole) return m_selectedBrush;
         }
      }
      return value;
   }
   bool setData(const QModelIndex &, const QVariant &, int) Q_DECL_OVERRIDE {
      return false;
   }
   void sort(int column, Qt::SortOrder order) Q_DECL_OVERRIDE {
      m_ignoreReset = true;
      QIdentityProxyModel::sort(column, order);
      m_ignoreReset = false;
   }
   void setSourceModel(QAbstractItemModel * model) Q_DECL_OVERRIDE {
      m_selection.clear();
      QIdentityProxyModel::setSourceModel(model);
   }
};

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);
   QWidget w;
   QVBoxLayout layout(&w);

   QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
   db.setDatabaseName(":memory:");
   if (! db.open()) return 255;

   QSqlQuery query(db);
   query.exec("create table chaps (name, age, constraint pk primary key (name, age));");
   query.exec("insert into chaps (name, age) values "
              "('Bob', 20), ('Rob', 30), ('Sue', 25), ('Hob', 40);");
   QSqlTableModel model(nullptr, db);
   model.setTable("chaps");

   RowSelectionEmulatorProxy proxy;
   proxy.setSourceModel(&model);
   proxy.setSelectedBrush(QBrush(Qt::yellow));

   QTableView view;
   view.setModel(&proxy);
   view.setEditTriggers(QAbstractItemView::NoEditTriggers);
   view.setSelectionMode(QAbstractItemView::NoSelection);
   view.setSortingEnabled(true);
   QObject::connect(&view, &QAbstractItemView::clicked, [&proxy](const QModelIndex & index){
      proxy.toggleRowSelection(index);
   });

   QPushButton clearSelection("Clear Selection");
   QObject::connect(&clearSelection, &QPushButton::clicked, [&proxy]{ proxy.clearSelection(); });

   layout.addWidget(&view);
   layout.addWidget(&clearSelection);
   w.show();
   app.exec();
}

#include "main.moc"
