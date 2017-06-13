// https://github.com/KubaO/stackoverflown/tree/master/questions/grid-fill-44485395
#include <QtWidgets>
#include <algorithm>
#include <cstdlib>

int GCD(int a, int b) {
   while (b) {
      auto r = a % b;
      a = b;
      b = r;
   }
   return a;
}

int rangeLCM(int begin, int end) {
   int lcm = 1;
   for (int i = begin; i < end; ++i)
      lcm = std::abs(lcm * i) / GCD(lcm, i);
   return lcm;
}

template <typename C> int LCM(const C & c) {
   return std::accumulate(c.begin(), c.end(), 1, [](int a, int b){
      return std::abs(a * b) / GCD(a, b);
   });
}

#if 0
template <typename T> void appendUnique(QVector<T> & dst, const T & src) {
   if (!dst.contains(src))
      dst.append(src);
}
#endif

template <typename T> void appendUnique(QVector<T> & dst, const QVector<T> & src) {
   for (auto & s : src)
      if (!dst.contains(s))
         dst.append(s);
}

struct OpenRange {
   int left = {}, right = {};
   OpenRange() = default;
   OpenRange(const OpenRange & o) { *this = o; }
   OpenRange(int left, int right) : left(left), right(right) { Q_ASSERT(left <= right); }
   bool operator==(const OpenRange & o) const {
      Q_ASSERT(left <= right && o.left <= o.right);
      return left == o.left && right == o.right;
   }
   OpenRange & operator=(const OpenRange & o) {
      Q_ASSERT(left <= right && o.left <= o.right);
      left = o.left;
      right = o.right;
      return *this;
   }
   OpenRange & operator&=(const OpenRange & o) {
      Q_ASSERT(left <= right && o.left <= o.right);
      left = qBound(left, std::max(left, o.left), right);
      right = qBound(left, std::min(right, o.right), right);
      return *this;

   }
   bool contains(const OpenRange & o) const {
      Q_ASSERT(left <= right && o.left <= o.right);
      return !o.size() || (left <= o.left && right >= o.right);
   }
   int size() const {
      Q_ASSERT(left <= right);
      return right - left;
   }
};

OpenRange operator&(OpenRange l, const OpenRange & r) { return l &= r; }


template <typename T> class Grid {
   int m_rows = {};
   int m_columns = {};
   QVector<T> m_data;
public:
   Grid() = default;
   Grid(int rows, int columns, const T & val = {}) :
      m_rows(rows), m_columns(columns), m_data(rows*columns, val) {}
   void add(int row, int col, int rowSpan, int colSpan, const T & val) {
      for (int r = row; r < row + rowSpan; ++r)
         for (int c = col; c < col + colSpan; ++c)
            at(r, c) = val;
   }
   T & at(int row, int col) { return m_data[col*m_rows + row]; }
   const T & at(int row, int col) const { return m_data.at(col*m_rows + row); }
   T get(int row, int col) const { return at(row, col); }
   OpenRange rowSpan(int row, int col) const {
      auto & val = at(row, col);
      OpenRange span(row, row+1);
      for (auto r = row+1; r < m_rows && val == at(r, col); ++r)
         span.right = r+1;
      for (auto r = row-1; r >= 0 && val == at(r, col); --r)
         span.left = r;
      return span;
   }
   OpenRange colSpan(int row, int col) const {
      auto & val = at(row, col);
      OpenRange span(col, col+1);
      for (auto c = col+1; c < m_columns && val == at(row, c); ++c)
         span.right = c+1;
      for (auto c = col-1; c >= 0 && val == at(row, c); --c)
         span.left = c;
      return span;
   }
   bool isEmpty() const { return m_data.isEmpty(); }
   bool operator==(const Grid & o) const {
      return m_rows == o.m_rows && m_columns == o.m_columns && m_data == o.m_data;
   }
   int rowCount() const { return m_rows; }
   int columnCount() const { return m_columns; }
   const QVector<T> & data() const { return m_data; }
   bool anyEmpty(int row, int col, int rowSpan, int colSpan) const {
      for (int r = row; r < row+rowSpan; ++r)
         for (int c = col; c < col+colSpan; ++c)
            if (at(r,c).begin() == at(r,c).end())
               return true;
      return false;
   }
};

QDebug operator<<(QDebug dbg, const Grid<QVector<int>> & g) {
   QDebugStateSaver saver(dbg);
   dbg.nospace() << "Grid{";
   for (int r = 0; r < g.rowCount(); ++r) {
      dbg << "[";
      for (int c = 0; c < g.columnCount(); ++c) {
         auto const & cell = g.at(r, c);
         if (cell.isEmpty())
            dbg << "-";
         else if (cell.size() == 1)
            dbg << cell[0];
         else {
            dbg << "(";
            for (int i = 0; i < cell.size(); ++i) {
               dbg << cell[i];
               if (i < cell.size() - 1) dbg << ",";
            }
            dbg << ")";
         }
         if (c < g.columnCount()-1) dbg << ",";
      }
      dbg << ((r < g.rowCount()-1) ? "], " : "]");
   }
   return dbg << "}";
}

class FillGridLayout : public QGridLayout {
   Q_OBJECT
   struct Item {
      QLayoutItem * item;
      int row, col, rowSpan, colSpan;
      bool isInvisible() const {
         return item && item->widget() && !item->widget()->isVisible();
      }
      // only spacers are owned by the underlying layout
      bool isOwned() const {
         return item && (item->layout() || item->widget());
      }
   };
   using Items = QVector<Item>;
   using Indexes = QVector<int>;

   int m_rows, m_columns; // original layout
   Items m_items;

   using AGrid = Grid<Indexes>;
   using Grids = QVector<AGrid>;

   Item getItemAt(int i) const {
      Item d;
      getItemPosition(i, &d.row, &d.col, &d.rowSpan, &d.colSpan);
      d.item = itemAt(i);
      return d;
   }
   Items peek() const {
      Items items;
      for (int i = 0; i < count(); ++i)
         items.append(getItemAt(i));
      return items;
   }
   Items take() {
      Items items;
      while (itemAt(0)) {
         auto item = getItemAt(0);
         items.append(item);
         takeAt(0);
      }
      return items;
   }
   AGrid getGrid() const {
      AGrid grid(m_rows, m_columns);
      int i = 0;
      for (auto & item : m_items) {
         if (!item.isInvisible())
            grid.add(item.row, item.col, item.rowSpan, item.colSpan, {i});
         i++;
      }
      return grid;
   }
   Grids findAllGrids(const AGrid & grid) {
      static int level;
      Grids grids(1, grid);
      ++level;
      for (int r = 0; r < m_rows; ++r)
         for (int c = 0; c < m_columns; ++c) {
            auto rowGrid = rowFillGrid(r, c, grid);
            auto colGrid = colFillGrid(r, c, grid);
            qDebug() << level << r << c << rowGrid << colGrid;
            if (!rowGrid.isEmpty())
               appendUnique(grids, findAllGrids(rowGrid));
            if (!colGrid.isEmpty())
               appendUnique(grids, findAllGrids(colGrid));
         }
      --level;
      return grids;
   }
   AGrid rowFillGrid(int row, int col, AGrid grid) const {
      auto indexes = grid.at(row, col);
      if (indexes.isEmpty())
         return {};
      auto const rowSpan = grid.rowSpan(row, col);
      auto const check = [this, row, rowSpan, &grid](int col){
         if (col < 0 || col >= m_columns)
            return false;
         auto next = grid.at(row, col);
         auto nextSpan = grid.rowSpan(row, col);
         return (next.isEmpty() && nextSpan.contains(rowSpan)) ||
               (!next.isEmpty() && nextSpan == rowSpan);
      };
      int left = col, right = col+1;
      for (; check(left-1); left--);
      for (; check(right); right++);
      if (!grid.anyEmpty(row, left, 1, right-left))
         return {};
      for (int c = left; c < right; c++)
         appendUnique(indexes, grid.at(row, c));
      grid.add(rowSpan.left, left, rowSpan.size(), right-left, indexes);
      return grid;
   }
   AGrid colFillGrid(int row, int col, AGrid grid) const {
      auto indexes = grid.at(row, col);
      if (indexes.isEmpty())
         return {};
      auto const colSpan = grid.colSpan(row, col);
      auto const check = [this, col, colSpan, &grid](int row){
         if (row < 0 || row >= m_rows)
            return false;
         auto next = grid.at(row, col);
         auto nextSpan = grid.colSpan(row, col);
         return (next.isEmpty() && nextSpan.contains(colSpan)) ||
               (!next.isEmpty() && nextSpan == colSpan);
      };
      int top = row, bottom = row+1;
      for (; check(top-1); top--);
      for (; check(bottom); bottom++);
      if (!grid.anyEmpty(top, col, bottom-top, 1))
         return {};
      for (int r = top; r < bottom; r++)
         appendUnique(indexes, grid.at(r, col));
      grid.add(top, colSpan.left, bottom-top, colSpan.size(), indexes);
      return grid;
   }

public:
   FillGridLayout(QWidget * parent = {}) : QGridLayout(parent) {}
   ~FillGridLayout() {
      return;
      for (auto & item : m_items)
         if (item.isOwned())
            delete item.item;
   }
   void convert() {
      m_rows = rowCount();
      m_columns = columnCount();
      m_items = peek();
      auto grid = getGrid();
      qDebug() << grid;
      qDebug() << findAllGrids(grid);
   }
   void refill() {

   }
};

class TestUi : public QWidget {
   QGridLayout m_layout{this};
   QGridLayout m_left;
   FillGridLayout m_right;
   QPushButton m_regen{"Regen"};
public:
   TestUi(int rows, int columns) {
      m_layout.addLayout(&m_left, 0, 0);
      m_layout.addLayout(&m_right, 0, 1);
      m_layout.addWidget(&m_regen, 1, 0);
      for (int i = 0; i < rows; i ++)
         for (int j = 0; j < columns; j++)
            add(i, j, rows);
      connect(&m_regen, &QPushButton::clicked, [this]{ m_right.convert(); });
   }
   void add(int r, int c, int rows) {
      auto text = QString::number(c*rows + r);
      auto button = new QPushButton{text};
      auto label = new QLabel{text};
      button->setCheckable(true);
      button->setChecked(true);
      label->setFrameStyle(QFrame::Box | QFrame::Plain);
      connect(button, &QPushButton::toggled, label, &QLabel::setVisible);
      m_left.addWidget(button, r, c);
      m_right.addWidget(label, r, c);
   }
};

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   TestUi ui(3, 3);
   ui.show();
   auto r = {1,2,3,4};
   qDebug() << LCM(r);
   return app.exec();
}
#include "main.moc"
