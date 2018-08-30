// https://github.com/KubaO/stackoverflown/tree/master/questions/filesystem-model-select-50132273
#include <QtWidgets>
#include <algorithm>

class DataOverlay : public QIdentityProxyModel {
   Q_OBJECT
   struct Cell {
      int role;
      QVariant value;
   };
   QVector<int> m_roles;
   QMap<QPersistentModelIndex, QVector<Cell>> m_data;
   QVariant cellValue(const QModelIndex &index, int role) const {
      auto it = m_data.find(index);
      if (it != m_data.end())
         for (auto &cell : *it)
            if (cell.role == role) return cell.value;
      return {};
   }
   QVariant &cellValue(const QModelIndex &index, int role) {
      auto &cells = m_data[index];
      for (auto &cell : cells)
         if (cell.role == role) return cell.value;
      cells.push_back({role, QVariant()});
      return cells.last().value;
   }
   bool check(const QModelIndex &index, int role) const {
      return index.isValid() && sourceModel() && m_roles.contains(role);
   }

  public:
   DataOverlay(QObject *parent = {}) : QIdentityProxyModel(parent) {}
   QVariant data(const QModelIndex &index, int role) const override {
      if (check(index, role)) return cellValue(index, role);
      return QIdentityProxyModel::data(index, role);
   }
   bool setData(const QModelIndex &index, const QVariant &value, int role) override {
      if (!check(index, role)) return QIdentityProxyModel::setData(index, value, role);
      cellValue(index, role) = value;
      return true;
   }
};

class SelectionOverlay : public QIdentityProxyModel {
   Q_OBJECT
   Q_PROPERTY(QItemSelectionModel *selectionModel READ selectionModel WRITE
                  setSelectionModel NOTIFY selectionModelChanged)
   Q_PROPERTY(QVector<int> roles READ roles WRITE setRoles)
   using self_t = SelectionOverlay;
   using base_t = QIdentityProxyModel;
   using model_t = QItemSelectionModel;
   QPointer<QItemSelectionModel> m_model;
   QVector<QMetaObject::Connection> m_modelConnections;
   QVector<int> m_roles{IsSelectedRole};
   bool m_rowSelection = true;
   bool m_uniqueSelection = false;
   bool m_flags = Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
   QModelIndex topLeft() const { return sourceModel()->index(0, 0); }
   QModelIndex bottomRight() const {
      return sourceModel()->index(sourceModel()->rowCount() - 1,
                                  sourceModel()->columnCount() - 1);
   }
   void onSelectionChanged(const QItemSelection &selected,
                           const QItemSelection &deselected) {
      auto delta = selected;
      delta.merge(deselected, QItemSelectionModel::Select);
      for (auto &range : qAsConst(delta)) {
         auto topLeft = mapFromSource(range.topLeft());
         auto bottomRight = mapFromSource(range.bottomRight());
         emit dataChanged(topLeft, bottomRight, m_roles);
      }
   }
   void onModelChanged(QAbstractItemModel *model) { setSourceModel(model); }
   bool check(const QModelIndex &index, int role) const {
      return index.isValid() && m_model && m_roles.contains(role);
   }

  public:
   static constexpr int IsSelectedRole = Qt::UserRole + 44;
   SelectionOverlay(QObject *parent = {}) : QIdentityProxyModel(parent) {}
   QItemSelectionModel *selectionModel() const { return m_model; }
   virtual void setSelectionModel(QItemSelectionModel *model) {
      if (model == m_model) return;
      for (auto &conn : m_modelConnections) disconnect(conn);
      m_model = model;
      m_modelConnections.clear();
      if (model) {
         m_modelConnections = {
             connect(m_model, &model_t::selectionChanged, this,
                     &self_t::onSelectionChanged),
             connect(m_model, &model_t::modelChanged, this, &self_t::onModelChanged),
             connect(m_model, &model_t::destroyed, this, [this] {
                setSelectionModel(nullptr);
                setSourceModel(nullptr);
             })};
         setSourceModel(model->model());
      }
      emit selectionModelChanged(m_model);
   }
   Q_SIGNAL void selectionModelChanged(QItemSelectionModel *);
   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
      if (!check(index, role)) return base_t::data(index, role);
      auto sel = m_model->isSelected(mapToSource(index));
      if (role == Qt::CheckStateRole) return sel ? 2 : 0;
      return sel;
   }
   bool setData(const QModelIndex &index, const QVariant &value,
                int role = Qt::EditRole) override {
      if (!check(index, role)) return base_t::setData(index, value, role);
      using Sel = QItemSelectionModel;
      Sel::SelectionFlags selMode = value.toBool() ? Sel::Select : Sel::Deselect;
      if (m_rowSelection) selMode |= Sel::Rows;
      if (m_uniqueSelection) selMode |= Sel::Current;
      m_model->select(mapToSource(index), selMode);
      return true;
   }
   QVector<int> roles() const { return m_roles; }
   void setRoles(QVector<int> roles) {
      std::sort(roles.begin(), roles.end());
      if (roles == m_roles) return;
      std::swap(roles, m_roles);
      if (!m_model) return;
      QVector<int> allRoles;
      std::merge(roles.begin(), roles.end(), m_roles.begin(), m_roles.end(),
                 std::back_inserter(allRoles));
      emit dataChanged(topLeft(), bottomRight(), allRoles);
   }
   Qt::ItemFlags flags(const QModelIndex &) const override {
      return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
   }
};

#include "main.moc"

int main(int argc, char **argv) {
   QApplication app{argc, argv};
   QWidget ui;
   QVBoxLayout layout{&ui};
   QTreeView left, right;
   layout.addWidget(&left);
   layout.addWidget(&right);

   QFileSystemModel model;
   SelectionOverlay selProxy;
   model.setRootPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
   auto rootPathIndex = model.index(model.rootPath());
   left.setModel(&model);
   left.setSelectionMode(QAbstractItemView::MultiSelection);
   left.setRootIndex(rootPathIndex);
   selProxy.setRoles({Qt::CheckStateRole});
   selProxy.setSelectionModel(left.selectionModel());
   right.setModel(&selProxy);
   right.setRootIndex(selProxy.mapFromSource(rootPathIndex));
   for (int col : {1, 2, 3}) right.hideColumn(col);

   ui.show();
   return app.exec();
}
