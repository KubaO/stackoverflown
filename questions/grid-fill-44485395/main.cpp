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

template <typename T> void appendUnique(QVector<T> & dst, const T & src) {
   if (!dst.contains(src))
      dst.append(src);
}

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
      for (int r = row; r < rowSpan; ++r)
         for (int c = col; c < colSpan; ++c)
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
};

class FillGridLayout : public QGridLayout {
   Q_OBJECT
   struct Item {
      QLayoutItem * item;
      // only spacers are owned by the underlying layout
      bool isOwned() const { return item->layout() || item->widget(); }
      int row, col, rowSpan, colSpan;
   };
   using Items = QVector<Item>;
   using Indexes = QVector<int>;

   int m_rows, m_columns; // original layout
   Items m_items;

   struct Fill {
      QVector<int> indexes;
      int row, col, rowSpan, colSpan;
      bool operator==(const Fill & o) const {
         return row == o.row && col == o.col
               && rowSpan == o.rowSpan && colSpan == o.colSpan && indexes == o.indexes;
      }
   };
   using Fills = QVector<Fill>;

   Items take() {
      Items items;
      while (itemAt(0)) {
         Item d;
         getItemPosition(0, &d.row, &d.col, &d.rowSpan, &d.colSpan);
         d.item = takeAt(0);
         items.append(d);
      }
      return items;
   }
   Fills itemsToFills(const Items & items) {
      Fills fills;
      int i = 0;
      for (auto & item : items) {
         Fill f;
         f.indexes.append(i++);
         f.row = item.row;
         f.col = item.col;
         f.rowSpan = item.rowSpan;
         f.colSpan = item.colSpan;
         fills.append(f);
      }
      return fills;
   }
   QVector<Fills> findAllFills(const Fills & fills) {
      QVector<Fills> all;
      for (int i = 0; i < fills.count(); ++i) {
         auto row = rowFill(i, fills);
         auto column = columnFill(i, fills);
         if (!row.isEmpty())
            appendUnique(all, findAllFills(row));
         if (!column.isEmpty())
            appendUnique(all, findAllFills(column));
      }
      return all;
   }
   int fillAt(int row, int col, const Fills & fills) {
      int i = 0;
      for (auto & f : fills) {
         if (f.row <= row && f.row + f.rowSpan >= row
             && f.col <= col && f.col + f.colSpan >= col)
            return i;
         i++;
      }
      return -1;
   }
   Fills rowFill(int f, Fills fills) {
      int row = fills.at(f).row;
      int col = fills.at(f).col;
      for (int i = col+1; i < m_columns; ++i) {
         int j = fillAt(row, i, fills);
         auto & can = fills.at(j);
         //if (can.rowSpan == )
      }
   }
   Fills columnFill(int f, Fills fills) {

   }

   using AGrid = Grid<Indexes>;
   using Grids = QVector<AGrid>;

   AGrid getGrid() const {
      AGrid grid(m_rows, m_columns);
      int i = 0;
      for (auto & item : m_items)
         grid.add(item.row, item.col, item.rowSpan, item.colSpan, {i++});
      return grid;
   }
   Grids findAllGrids(const AGrid & grid) {
      Grids grids;
      for (int r = 0; r < m_rows; ++r)
         for (int c = 0; c < m_columns; ++c) {
            auto row = rowFillGrid(r, c, grid);
            auto col = colFillGrid(r, c, grid);
            if (!row.isEmpty())
               appendUnique(grids, findAllGrids(grid));
            if (!col.isEmpty())
               appendUnique(grids, findAllGrids(grid));
         }
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
      int left = col, right = col;
      for (; check(left-1); left--)
         appendUnique(indexes, grid.at(row, left-1));
      for (; check(right+1); right++)
         appendUnique(indexes, grid.at(row, right+1));
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
      int top = row, bottom = row;
      for (; check(top-1); top--)
         appendUnique(indexes, grid.at(top-1, col));
      for (; check(bottom+1); bottom++)
         appendUnique(indexes, grid.at(bottom+1, col));
      grid.add(top, colSpan.left, bottom-top, colSpan.size(), indexes);
      return grid;
   }

public:
   FillGridLayout(QWidget * parent = {}) : QGridLayout(parent) {}
   ~FillGridLayout() {
      for (auto & item : m_items)
         if (item.isOwned())
            delete item.item;
   }
   void convert() {
      m_rows = rowCount();
      m_columns = columnCount();
      m_items = take();
   }
   void refill() {

   }
};

class TestUi : public QWidget {
   QHBoxLayout m_layout{this};
   QGridLayout m_left;
   FillGridLayout m_right;
public:
   TestUi(int rows, int columns) {
      m_layout.addLayout(&m_left);
      m_layout.addLayout(&m_right);
      for (int i = 0; i < rows; i ++)
         for (int j = 0; j < columns; j++)
            add(i, j, rows);
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
   TestUi ui(3, 4);
   ui.show();
   auto r = {1,2,3,4};
   qDebug() << LCM(r);
   return app.exec();
}
#include "main.moc"
