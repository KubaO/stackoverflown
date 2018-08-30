// https://github.com/KubaO/stackoverflown/tree/master/questions/transparent-proxy-19835618
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

class TransparentProxyModel : public QAbstractProxyModel {
   Q_OBJECT
   struct Helper : QAbstractItemModel {
      using QAbstractItemModel::createIndex;
   };
   struct Op {
      enum Kind { AddRow, RemoveRow, AddCol, RemoveCol, MoveRow, MoveCol } kind;
      QModelIndex parentSrc;
      int first, last;
      QModelIndex parentDst;
      int index;
      bool checkSrc(Kind k, const QModelIndex &i, int f, int l) const {
         return kind == k && parentSrc == i && first == f && last == l;
      }
      bool checkDst(const QModelIndex &i, int n) const {
         return parentDst == i && index == n;
      }
   };
   QVector<Op> m_addsRemoves;
   QModelIndex createSourceIndex(int r, int c, void *data) const {
      return static_cast<Helper *>(sourceModel())->createIndex(r, c, data);
   }
   Q_SLOT void onDataChanged(const QModelIndex &tl, const QModelIndex &br) {
      emit dataChanged(mapFromSource(tl), mapFromSource(br));
   }
   Q_SLOT void onRowsAboutToBeInserted(const QModelIndex &parent, int first, int last) {
      m_addsRemoves.push_back({Op::AddRow, parent, first, last});
      beginInsertRows(mapFromSource(parent), first, last);
   }
   Q_SLOT void onRowsInserted(const QModelIndex &parent, int first, int last) {
      Q_ASSERT(!m_addsRemoves.isEmpty());
      Q_ASSERT(m_addsRemoves.last().checkSrc(Op::AddRow, parent, first, last));
      m_addsRemoves.pop_back();
      endInsertRows();
   }
   Q_SLOT void onRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last) {
      m_addsRemoves.push_back({Op::RemoveRow, parent, first, last});
      beginRemoveRows(mapFromSource(parent), first, last);
   }
   Q_SLOT void onRowsRemoved(const QModelIndex &parent, int first, int last) {
      Q_ASSERT(!m_addsRemoves.isEmpty());
      Q_ASSERT(m_addsRemoves.last().checkSrc(Op::RemoveRow, parent, first, last));
      m_addsRemoves.pop_back();
      endRemoveRows();
   }
   Q_SLOT void onColumnsAboutToBeInserted(const QModelIndex &parent, int first,
                                          int last) {
      m_addsRemoves.push_back({Op::AddCol, parent, first, last});
      beginInsertColumns(parent, first, last);
   }
   Q_SLOT void onColumnsInserted(const QModelIndex &parent, int first, int last) {
      Q_ASSERT(!m_addsRemoves.isEmpty());
      Q_ASSERT(m_addsRemoves.last().checkSrc(Op::AddCol, parent, first, last));
      m_addsRemoves.pop_back();
      endInsertColumns();
   }
   Q_SLOT void onColumnsAboutToBeRemoved(const QModelIndex &parent, int first, int last) {
      m_addsRemoves.push_back({Op::RemoveCol, parent, first, last});
      beginRemoveColumns(mapFromSource(parent), first, last);
   }
   Q_SLOT void onColumnsRemoved(const QModelIndex &parent, int first, int last) {
      Q_ASSERT(!m_addsRemoves.isEmpty());
      Q_ASSERT(m_addsRemoves.last().checkSrc(Op::RemoveCol, parent, first, last));
      m_addsRemoves.pop_back();
      endRemoveColumns();
   }
   Q_SLOT void onRowsAboutToBeMoved(const QModelIndex &srcParent, int start, int end,
                                    const QModelIndex &dstParent, int row) {
      m_addsRemoves.push_back({Op::MoveRow, srcParent, start, end, dstParent, row});
      beginMoveRows(mapFromSource(srcParent), start, end, mapFromSource(dstParent), row);
   }
   Q_SLOT void onRowsMoved(const QModelIndex &srcParent, int start, int end,
                           const QModelIndex &dstParent, int row) {
      Q_ASSERT(!m_addsRemoves.isEmpty());
      auto const &op = m_addsRemoves.last();
      Q_ASSERT(op.checkSrc(Op::MoveRow, srcParent, start, end) &&
               op.checkDst(dstParent, row));
      m_addsRemoves.pop_back();
      endMoveRows();
   }
   Q_SLOT void onColumnsAboutToBeMoved(const QModelIndex &srcParent, int start, int end,
                                       const QModelIndex &dstParent, int col) {
      m_addsRemoves.push_back({Op::MoveCol, srcParent, start, end, dstParent, col});
      beginMoveColumns(mapFromSource(srcParent), start, end, mapFromSource(dstParent),
                       col);
   }
   Q_SLOT void onColumnsMoved(const QModelIndex &srcParent, int start, int end,
                              const QModelIndex &dstParent, int col) {
      Q_ASSERT(!m_addsRemoves.isEmpty());
      auto const &op = m_addsRemoves.last();
      Q_ASSERT(op.checkSrc(Op::MoveRow, srcParent, start, end) &&
               op.checkDst(dstParent, col));
      m_addsRemoves.pop_back();
      endMoveColumns();
   }

  public:
   TransparentProxyModel(QObject *parent = nullptr) : QAbstractProxyModel(parent) {}
   QModelIndex mapFromSource(const QModelIndex &src) const override {
      if (!src.isValid() || !sourceModel()) return {};
      Q_ASSERT(src.model() == sourceModel());
      return createIndex(src.row(), src.column(), src.internalPointer());
   }
   QModelIndex mapToSource(const QModelIndex &prx) const override {
      if (!prx.isValid() || !sourceModel()) return {};
      Q_ASSERT(prx.model() == this);
      return createSourceIndex(prx.row(), prx.column(), prx.internalPointer());
   }
   QModelIndex index(int row, int column, const QModelIndex &parent) const override {
      if (!sourceModel()) return {};
      Q_ASSERT(!parent.isValid() || parent.model() == this);
      return mapFromSource(sourceModel()->index(row, column, mapToSource(parent)));
   }
   int rowCount(const QModelIndex &parent) const override {
      if (!sourceModel()) return 0;
      Q_ASSERT(!parent.isValid() || parent.model() == this);
      return sourceModel()->rowCount(mapToSource(parent));
   }
   int columnCount(const QModelIndex &parent) const override {
      if (!sourceModel()) return 0;
      Q_ASSERT(!parent.isValid() || parent.model() == this);
      return sourceModel()->columnCount(mapToSource(parent));
   }
   QModelIndex parent(const QModelIndex &child) const override {
      if (!child.isValid() || !sourceModel()) return {};
      Q_ASSERT(child.model() == this);
      return mapFromSource(sourceModel()->parent(mapToSource(child)));
   }
   void setSourceModel(QAbstractItemModel *model) override {
      if (sourceModel()) disconnect(sourceModel(), 0, this, 0);
      QAbstractProxyModel::setSourceModel(model);
      if (!sourceModel()) return;
      connect(model, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this,
              SLOT(onDataChanged(QModelIndex, QModelIndex)));
      connect(model, SIGNAL(headerDataChanged(Qt::Orientation, int, int)), this,
              SIGNAL(headerDataChanged(Qt::Orientation, int, int)));
      connect(model, SIGNAL(layoutChanged()), this, SIGNAL(layoutChanged()));
      connect(model, SIGNAL(layoutAboutToBeChanged()), this,
              SIGNAL(layoutAboutToBeChanged()));
      connect(model, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)), this,
              SLOT(onRowsAboutToBeInserted(QModelIndex, int, int)));
      connect(model, SIGNAL(rowsInserted(QModelIndex, int, int)), this,
              SLOT(onRowsInserted(QModelIndex, int, int)));
      connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)), this,
              SLOT(onRowsAboutToBeRemoved(QModelIndex, int, int)));
      connect(model, SIGNAL(rowsRemoved(QModelIndex, int, int)), this,
              SLOT(onRowsRemoved(QModelIndex, int, int)));
      connect(model, SIGNAL(columnsAboutToBeInserted(QModelIndex, int, int)), this,
              SLOT(onColumnsAboutToBeInserted(QModelIndex, int, int)));
      connect(model, SIGNAL(columnsInserted(QModelIndex, int, int)), this,
              SLOT(onColumnsInserted(QModelIndex, int, int)));
      connect(model, SIGNAL(columnsAboutToBeRemoved(QModelIndex, int, int)), this,
              SLOT(onColumnsAboutToBeRemoved(QModelIndex, int, int)));
      connect(model, SIGNAL(columnsRemoved(QModelIndex, int, int)), this,
              SLOT(onColumnsRemoved(QModelIndex, int, int)));
      connect(model, SIGNAL(modelAboutToBeReset()), this, SIGNAL(modelAboutToBeReset()));
      connect(model, SIGNAL(modelReset()), this, SIGNAL(modelReset()));
      connect(model, SIGNAL(rowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)),
              this, SLOT(onRowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)));
      connect(model, SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)), this,
              SLOT(onRowsMoved(QModelIndex, int, int, QModelIndex, int)));
      connect(
          model, SIGNAL(columnsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)),
          this, SLOT(onColumnsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)));
      connect(model, SIGNAL(columnsMoved(QModelIndex, int, int, QModelIndex, int)), this,
              SLOT(onColumnsMoved(QModelIndex, int, int, QModelIndex, int)));
   }
};

int main(int argc, char *argv[]) {
   QApplication app(argc, argv);
   QFileSystemModel model;
   TransparentProxyModel proxy;
   proxy.setSourceModel(&model);
   QTreeView view;
   view.setModel(&proxy);
   model.setRootPath(QDir::homePath());
   view.setRootIndex(proxy.mapFromSource(model.index(QDir::homePath())));
   view.show();
   return app.exec();
}
#include "main.moc"
