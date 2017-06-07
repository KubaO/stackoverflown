// https://github.com/KubaO/stackoverflown/tree/master/questions/paged-viewmodel-44375460
#include <QtWidgets>

/// A paged view of a table.
class PagedTableViewModel : public QAbstractProxyModel {
   Q_OBJECT
   Q_PROPERTY(int page READ page WRITE setPage NOTIFY pageChanged)
   Q_PROPERTY(int pageLength READ pageLength WRITE setPageLength NOTIFY pageLengthChanged)
   int m_pageRow = 0;
   int m_pageLength = 20;
   int m_rowCountAdjustment = 0;
protected:
   void moveRows(int src, int count, int dst, bool assertInternal = true) {
      auto adjDst = src > dst ? dst : dst - count;
      auto internal = src >= 0 && (src+count) <= m_pageLength
            && adjDst >= 0 && (adjDst+count) <= m_pageLength;
      if (assertInternal)
         Q_ASSERT(internal);
      if (internal) {
         auto result = beginMoveRows({}, src, src + count - 1, {}, dst);
         Q_ASSERT(result);
      }
      auto result = sourceModel()->moveRows({}, m_pageRow + src, count, {}, m_pageRow + dst);
      Q_ASSERT(result);
      if (internal)
         endMoveRows();
   }
public:
   PagedTableViewModel(QObject * parent = {}) : QAbstractProxyModel(parent) {}
   explicit PagedTableViewModel(int pageLength, QObject * parent = {}) :
      QAbstractProxyModel(parent),  m_pageLength(pageLength) {}
   void setSourceModel(QAbstractItemModel *sourceModel) override {
      beginResetModel();
      QAbstractProxyModel::setSourceModel(sourceModel);
      setPage(0);
      endResetModel();
   }
   QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override {
      if (!sourceModel())
         return {};
      auto row = sourceIndex.row();
      if (row < m_pageRow || row >= m_pageRow + m_pageLength)
         return {};
      return index(row - m_pageRow, sourceIndex.column(), {});
   }
   QModelIndex mapToSource(const QModelIndex &proxyIndex) const override {
      if (!sourceModel())
         return {};
      return sourceModel()->index(proxyIndex.row() + m_pageRow, proxyIndex.column());
   }
   int rowCount(const QModelIndex &parent = {}) const override {
      if (!sourceModel() || parent.isValid())
         return 0;
      int count = std::max(0, sourceModel()->rowCount() - m_pageRow);
      count = std::min(count, m_pageLength);
      count += m_rowCountAdjustment;
      Q_ASSERT(count >= 0);
      return count;
   }
   int columnCount(const QModelIndex &parent = {}) const override {
      if (!sourceModel() || parent.isValid())
         return 0;
      return sourceModel()->columnCount();
   }
   QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override {
      if (!sourceModel() || parent.isValid()
          || row < 0 || column < 0)
         return {};
      return createIndex(row, column);
   }
   QModelIndex parent(const QModelIndex &child) const override {
      Q_UNUSED(child);
      return {};
   }
   int pageLength() const { return m_pageLength; }
   void setPageLength(int pageLength) {
      Q_ASSERT(pageLength);
      if (pageLength == m_pageLength)
         return;
      beginResetModel();
      m_pageLength = pageLength;
      emit pageLengthChanged(m_pageLength);
      endResetModel();
   }
   int page() const { return m_pageLength ? m_pageRow / m_pageLength : 0; }
   void setPage(int page) {
      if (page < 0)
         return;
      auto pageRow = page * m_pageLength;
      if (pageRow == m_pageRow)
         return;
      beginResetModel();
      m_pageRow = pageRow;
      emit pageChanged(page);
      endResetModel();
   }
   Q_SIGNAL void pageChanged(int page);
   Q_SIGNAL void pageLengthChanged(int pageLength);
   Q_SLOT void nextPage() { setPage(page() + 1); }
   Q_SLOT void prevPage() { setPage(page() - 1); }
   bool insertRows(int row, int count, const QModelIndex &parent = {}) override;
   bool removeRows(int row, int count, const QModelIndex &parent = {}) override;
   bool moveRows(const QModelIndex &sourceParent, int src, int count,
                 const QModelIndex &destinationParent, int dst) override;
};

bool PagedTableViewModel::insertRows(int row, int count, const QModelIndex &parent) {
   if (!sourceModel() || parent.isValid() || row < 0 || row > rowCount())
      return false;
   int excess = std::max(0, rowCount()+count-m_pageLength);
   beginInsertRows({}, row, row+count-1);
   auto result = sourceModel()->insertRows(m_pageRow + row, count);
   Q_ASSERT(result);
   m_rowCountAdjustment = excess;
   endInsertRows();
   if (excess) {
      beginRemoveRows({}, m_pageLength, m_pageLength + excess - 1);
      m_rowCountAdjustment = 0;
      endRemoveRows();
   }
   Q_ASSERT(!m_rowCountAdjustment);
   return true;
}

bool PagedTableViewModel::removeRows(int row, int count, const QModelIndex &parent) {
   if (!sourceModel() || parent.isValid() || row < 0 || row >= rowCount())
      return false;
   count = std::min(count, rowCount() - row);
   if (count <= 0)
      return false;
   bool onlyRemove = row + count >= rowCount();
   if (onlyRemove)
      beginRemoveRows({}, row, row+count-1);
   else {
      auto allow = beginMoveRows({}, row+count, rowCount()-1, {}, row);
      Q_ASSERT(allow);
   }
   m_rowCountAdjustment = -count;
   bool result = sourceModel()->removeRows(m_pageRow + row, count);
   Q_ASSERT(result); // TODO: support undoing the removal
   if (onlyRemove)
      endRemoveRows();
   else
      endMoveRows();
   auto prevRowCount = rowCount();
   m_rowCountAdjustment = 0;
   auto added = rowCount() - prevRowCount;
   if (added) {
      m_rowCountAdjustment = -count;
      beginInsertRows({}, rowCount(), rowCount()+added-1);
      m_rowCountAdjustment = 0;
      endInsertRows();
   }
   Q_ASSERT(!m_rowCountAdjustment);
   return true;
}

bool PagedTableViewModel::moveRows(const QModelIndex &sourceParent, int src, int count,
                                   const QModelIndex &destinationParent, int dst) {
   auto const adjDst = src > dst ? dst : dst - count;
   if (!sourceModel() || count < 0 || sourceParent.isValid() || destinationParent.isValid()
       || src < 0 || src+count > rowCount()
       || (m_pageRow+adjDst) < 0 || (m_pageRow+adjDst) > sourceModel()->rowCount())
      return false;
   if (count == 0)
      return true;
   if (adjDst >= 0 && (adjDst+count) <= rowCount()) {
      // Destination fits within the page
      moveRows(src, count, dst);
   } else if (dst < 0) {
      // Destination is partially before the page
      int excess = -dst;
      int first = src;
      int last = count - excess;
      Q_ASSERT(excess > 0);
      //                                      **************
      // Layout [~excess~] (m_pageRow) [first][excess][last]
      if (first)
         moveRows(0, first, first + excess + last);
      // Layout [~excess~] (m_pageRow) [excess][last][first]
      moveRows(-excess, excess, excess, false);
      emit dataChanged(index(0, 0), index(excess - 1, 0));
      // Layout [excess] (m_pageRow) [~excess~][last][first]
      if (last)
         moveRows(excess, last, 0);
      //        ***************************
      // Layout [excess] (m_pageRow) [last][~excess~][first]
   } else {
      // Destination is partially after the page
      int excess = dst - m_pageLength;
      int first = count - excess;
      int last = m_pageLength - src - count;
      Q_ASSERT(excess > 0);
      //        ***************
      // Layout [first][excess][last] (nextPageRow) [~excess~]
      if (last)
         moveRows(src + count, last, src);
      // Layout [last][first][excess] (nextPageRow) [~excess~]
      moveRows(m_pageLength, excess, m_pageLength-excess, false);
      emit dataChanged(index(m_pageLength-excess, 0), index(m_pageLength-1, 0));
      // Layout [last][first][~excess] (nextPageRow) [excess]
      if (first)
         moveRows(m_pageLength - excess, excess, m_pageLength - excess - first);
      //                        ******************************
      // Layout [last][~excess~][first] (nextPageRow) [excess]
   }
   return true;
}

class Ui : public QWidget {
   Q_OBJECT
   int m_addCount = 0;
   QGridLayout m_layout{this};
   QPushButton m_prev{"<"};
   QLabel m_page{"1"};
   QPushButton m_next{">"};
   QTableView m_view;
   QPushButton m_up{"Move up"};
   QPushButton m_down{"Move down"};
   QPushButton m_remove{"Remove"};
   QSpinBox m_count;
   QPushButton m_before{"Insert Before"};
   QPushButton m_after{"Insert After"};
   QAbstractItemModel * model() const { return m_view.model(); }
   const QItemSelection selection() const {
      auto model = m_view.selectionModel();
      return model ? model->selection() : QItemSelection();
   }
   int firstSelectedRow() const {
      return !selection().isEmpty() ? selection().indexes().first().row() : -1;
   }
   int lastSelectedRow() const {
      return !selection().isEmpty() ? selection().indexes().last().row() : -1;
   }
   int selectionSize() const {
      return selection().indexes().size();
   }
   void addRow(int row, int offset) {
      if (!model() || row < 0) return;
      row += offset;
      for (auto n = m_count.value(); n; n--, row++)
         if (model()->insertRow(row))
            model()->setData(model()->index(row, 0),
                             QStringLiteral("Added %1").arg(++m_addCount));
   }
   void moveRows(int direction) {
      if (!model() || firstSelectedRow() < 0) return;
      auto moveBy = m_count.value();
      auto offset = (direction < 0) ? -moveBy : moveBy + selectionSize();
      model()->moveRows({}, firstSelectedRow(), selectionSize(),
                        {}, firstSelectedRow() + offset);
   }
public:
   Ui() {
      m_view.setSelectionMode(QAbstractItemView::ContiguousSelection);
      m_layout.addWidget(&m_prev, 0, 0);
      m_layout.addWidget(&m_page, 0, 1);
      m_layout.addWidget(&m_next, 0, 2);
      m_layout.addWidget(&m_view, 1, 0, 1, 3);
      m_layout.addWidget(&m_up, 2, 0);
      m_layout.addWidget(&m_down, 3, 0);
      m_layout.addWidget(&m_remove, 2, 1);
      m_layout.addWidget(&m_count, 3, 1);
      m_layout.addWidget(&m_before, 2, 2);
      m_layout.addWidget(&m_after, 3, 2);
      m_count.setValue(1);
      m_count.setMinimum(1);
      m_count.setMaximum(5);
      m_count.setPrefix("# ");
      connect(&m_prev, &QPushButton::clicked, this, &Ui::prevPage);
      connect(&m_next, &QPushButton::clicked, this, &Ui::nextPage);
      connect(&m_remove, &QPushButton::clicked, this, [this]{
         if (!model()) return;
         if (firstSelectedRow() >= 0)
            model()->removeRows(firstSelectedRow(), selectionSize());
      });
      connect(&m_before, &QPushButton::clicked, this, [this]{ addRow(firstSelectedRow(), 0); });
      connect(&m_after, &QPushButton::clicked, this, [this]{ addRow(lastSelectedRow(), 1); });
      connect(&m_up, &QPushButton::clicked, this, [this]{ moveRows(-1); });
      connect(&m_down, &QPushButton::clicked, this, [this]{ moveRows(+1); });
   }
   void setModel(QAbstractItemModel * model) {
      m_view.setModel(model);
   }
   Q_SIGNAL void nextPage();
   Q_SIGNAL void prevPage();
   Q_SLOT void onPageChanged(int page) {
      m_page.setNum(page + 1);
   }
};

// see https://stackoverflow.com/a/7533658/1329652
template <typename C>
void move_range(size_t start, size_t length, size_t dst, C & data)
{
   typename C::iterator first, middle, last;
   if (start < dst)
   {
      first  = data.begin() + start;
      middle = first + length;
      last   = data.begin() + dst;
   }
   else
   {
      first  = data.begin() + dst;
      middle = data.begin() + start;
      last   = middle + length;
   }
   std::rotate(first, middle, last);
}

class StringListModel : public QAbstractListModel {
   Q_OBJECT
   QStringList m_list;
public:
   void setList(const QStringList & list) {
      beginResetModel();
      m_list = list;
      endResetModel();
   }
   int rowCount(const QModelIndex &parent = {}) const override {
      return !parent.isValid() ? m_list.count() : 0;
   }
   QVariant data(const QModelIndex &index, int role) const override {
      if (index.parent().isValid() || index.column() != 0
          || index.row() < 0 || index.row() >= rowCount()
          || (role != Qt::DisplayRole && role != Qt::EditRole))
         return {};
      return m_list[index.row()];
   }
   bool setData(const QModelIndex &index, const QVariant &value, int role) override {
      if (index.parent().isValid() || index.column() != 0
          || index.row() < 0 || index.row() >= rowCount()
          || (role != Qt::DisplayRole && role != Qt::EditRole))
         return false;
      m_list[index.row()] = value.toString();
      return true;
   }
   Qt::ItemFlags flags(const QModelIndex &index) const override {
      auto flags = QAbstractListModel::flags(index) | Qt::ItemIsDropEnabled;
      return index.isValid() ? flags |  Qt::ItemIsEditable | Qt::ItemIsDragEnabled : flags;
   }
   bool insertRows(int row, int count, const QModelIndex &parent) override {
      if (parent.isValid() || row > rowCount())
         return false;
      beginInsertRows({}, row, row+count-1);
      while (count--)
         m_list.insert(row, {});
      endInsertRows();
      return true;
   }
   bool removeRows(int row, int count, const QModelIndex &parent) override {
      if (parent.isValid() || row >= rowCount())
         return false;
      beginRemoveRows({}, row, row+count-1);
      while (count--)
         m_list.removeAt(row);
      endRemoveRows();
      return true;
   }
   bool moveRows(const QModelIndex &sourceParent, int src, int const count,
                 const QModelIndex &destinationParent, int dst) override {
      if (sourceParent.isValid() || destinationParent.isValid()
          || src < 0 || src + count > rowCount()
          || dst < 0 || dst > rowCount()
          || !beginMoveRows({}, src, src+count-1, {}, dst))
         return false;
      move_range(src, count, dst, m_list);
      endMoveRows();
      return true;
   }
};

int main(int argc, char ** argv) {
   using Q = QObject;
   QApplication app{argc, argv};
   StringListModel model;
   PagedTableViewModel viewModel;
   viewModel.setSourceModel(&model);
   viewModel.setPageLength(5);
   QStringList list;
   for (int i = 0; i < 1000; ++i)
      list << QStringLiteral("Item %1").arg(i+1);
   model.setList(list);

   Ui ui;
   ui.setModel(&viewModel);
   Q::connect(&ui, &Ui::nextPage, &viewModel, &PagedTableViewModel::nextPage);
   Q::connect(&ui, &Ui::prevPage, &viewModel, &PagedTableViewModel::prevPage);
   Q::connect(&viewModel, &PagedTableViewModel::pageChanged, &ui, &Ui::onPageChanged);
   ui.show();
   return app.exec();
}
#include "main.moc"
