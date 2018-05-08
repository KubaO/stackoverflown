// https://github.com/KubaO/stackoverflown/tree/master/questions/filesystem-model-select-50132273
#include <QtWidgets>
#include <algorithm>

class SelectionProxy : public QIdentityProxyModel {
   Q_OBJECT
   Q_PROPERTY(QItemSelectionModel* selectionModel
              READ selectionModel WRITE setSelectionModel NOTIFY selectionModelChanged)
   Q_PROPERTY(QVector<int> roles READ roles WRITE setRoles)
   using self_t = SelectionProxy;
   using base_t = QIdentityProxyModel;
   using model_t = QItemSelectionModel;
   QPointer<QItemSelectionModel> m_model;
   QVector<QMetaObject::Connection> m_modelConnections;
   QVector<int> m_roles{IsSelectedRole};
   QModelIndex topLeft() const {
      return sourceModel()->index(0, 0);
   }
   QModelIndex bottomRight() const {
      return sourceModel()->index(sourceModel()->rowCount()-1, sourceModel()->columnCount()-1);
   }
   void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
      auto sel = selected;
      sel.merge(deselected, QItemSelectionModel::Select);
      for (auto &range : qAsConst(sel)) {
         auto topLeft = mapFromSource(range.topLeft());
         auto bottomRight = mapFromSource(range.bottomRight());
         emit dataChanged(topLeft, bottomRight, m_roles);
      }
   }
   void onModelChanged(QAbstractItemModel *model) {
      setSourceModel(model);
   }
   bool check(const QModelIndex &index, int role) const {
      return index.isValid() && m_model && m_roles.contains(role);
   }
public:
   static constexpr int IsSelectedRole = Qt::UserRole + 44;
   SelectionProxy(QObject *parent = {}) : QIdentityProxyModel(parent) {}
   QItemSelectionModel *selectionModel() const { return m_model; }
   virtual void setSelectionModel(QItemSelectionModel *model) {
      if (model == m_model) return;
      for (auto &conn : m_modelConnections)
         disconnect(conn);
      m_model = model;
      m_modelConnections = {
         connect(m_model, &model_t::selectionChanged, this, &self_t::onSelectionChanged),
         connect(m_model, &model_t::modelChanged, this, &self_t::onModelChanged) };
      setSourceModel(model->model());
      emit selectionModelChanged(m_model);
   }
   Q_SIGNAL void selectionModelChanged(QItemSelectionModel *);
   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
      if (!check(index, role))
         return base_t::data(index, role);
      return m_model->isSelected(mapToSource(index));
   }
   bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override {
      if (!check(index, role))
         return base_t::setData(index, value, role);
      using Sel = QItemSelectionModel;
      m_model->select(mapToSource(index), value.toBool() ? Sel::SelectCurrent : (Sel::Deselect | Sel::Current));
      return true;
   }
   QVector<int> roles() const { return m_roles; }
   void setRoles(QVector<int> roles) {
      std::sort(roles.begin(), roles.end());
      if (roles == m_roles)
         return;
      std::swap(roles, m_roles);
      if (!m_model)
         return;
      QVector<int> allRoles;
      std::merge(roles.begin(), roles.end(), m_roles.begin(), m_roles.end(), std::back_inserter(allRoles));
      emit dataChanged(topLeft(), bottomRight(), allRoles);
   }
   void setRole(int role) {
      setRoles({role});
   }
};

#include "main.moc"

int main(int argc, char **argv) {
   QApplication app{argc, argv};
   QWidget win;
   QVBoxLayout layout{&win};
   QTreeView left, right;
   layout.addWidget(&left);
   layout.addWidget(&right);

   QFileSystemModel model;
   SelectionProxy selProxy;
   model.setRootPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
   auto rootPathIndex = model.index(model.rootPath());
   left.setModel(&model);
   left.setSelectionMode(QAbstractItemView::MultiSelection);
   left.setRootIndex(rootPathIndex);
   selProxy.setRole(Qt::DisplayRole);
   selProxy.setSelectionModel(left.selectionModel());
   right.setModel(&selProxy);
   right.setRootIndex(selProxy.mapFromSource(rootPathIndex));
   for (int col : {1,2,3})
      right.hideColumn(col);

   win.show();
   return app.exec();
}
