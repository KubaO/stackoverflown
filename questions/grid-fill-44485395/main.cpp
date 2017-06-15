// https://github.com/KubaO/stackoverflown/tree/master/questions/grid-fill-44485395
#include <QtWidgets>
#include <algorithm>
#include <cstdlib>
#include <set>

int GCD(int a, int b) {
   while (b) {
      auto r = a % b;
      a = b;
      b = r;
   }
   return a;
}

template <typename I1, typename I2> int LCM(I1 begin, I2 end) {
   return std::accumulate(begin, end, 1, [](int a, int b){
      return std::abs(a * b) / GCD(a, b);
   });
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
   OpenRange(int left, int right) : left(left), right(right) {}
   bool operator==(const OpenRange & o) const {
      return left == o.left && right == o.right;
   }
   OpenRange & operator=(const OpenRange & o) {
      left = o.left;
      right = o.right;
      return *this;
   }
   OpenRange & operator&=(const OpenRange & o) {
      left = qBound(left, std::max(left, o.left), right);
      right = qBound(left, std::min(right, o.right), right);
      return *this;
   }
   bool contains(const OpenRange & o) const {
      return !o.size() || (left <= o.left && right >= o.right);
   }
   int size() const {
      return right - left;
   }
   OpenRange scaled(int factor) const {
      return { left*factor, right*factor };
   }
   struct iterator {
      friend struct OpenRange;
      int value;
      operator int() const { return value; }
      int operator*() const { return value; }
      iterator & operator++() { value += 1; return *this; }
      bool operator==(iterator o) const { return value == o.value; }
   private:
      iterator(int value) : value(value) {}
   };
   iterator begin() const { return left; }
   iterator end() const { return right; }
};

OpenRange operator&(OpenRange l, const OpenRange & r) { return l &= r; }

struct Fill : std::set<int> {
   enum class Type { None, Row, Column } type = Type::None;
   Fill() = default;
   Fill(const Fill &) = default;
   Fill(Fill &&) = default;
   Fill(int index) : std::set<int>{index} {}
   Fill & operator=(const Fill &) = default;
   Fill & operator=(Fill &&) = default;
   Type otherType() const {
      return (type == Type::Row) ? Type::Column : (type == Type::Column) ? Type::Row : type;
   }
   bool operator==(const Fill & o) const {
      return o.type == type && o == static_cast<const std::set<int>&>(*this);
   }
};

void insert(Fill & dst, const Fill & src) {
   dst.insert(src.begin(), src.end());
}

struct GridItem {
   Fill fill;
   OpenRange rows, columns;
   int denominator = 1;
   int row() const { return rows.left; }
   int column() const { return columns.left; }
   int rowSpan() const { return rows.size(); }
   int colSpan() const { return columns.size(); }
   int index() const { Q_ASSERT(fill.size() == 1); return *fill.begin(); }
   bool empty() const { return fill.empty(); }
   const OpenRange & range(Fill::Type type) const {
      Q_ASSERT(type != Fill::Type::None);
      return type == Fill::Type::Row ? rows : columns;
   }
   OpenRange & range(Fill::Type type) {
      Q_ASSERT(type != Fill::Type::None);
      return type == Fill::Type::Row ? rows : columns;
   }
   GridItem() = default;
   GridItem(const Fill & fill, const OpenRange & rows, const OpenRange & cols) :
      fill(fill), rows(rows), columns(cols) {}
   QVector<GridItem> split() const {
      if (empty())
         return {};
      if (fill.type == Fill::Type::None || fill.size() == 1)
         return {*this}; // nothing to split
      QVector<GridItem> items;
      auto item = *this;
      item.denominator = item.fill.size();
      auto const fill = item.fill;
      auto & range = item.range(fill.otherType());
      auto const size = range.size();
      auto pos = range.left * item.denominator;
      for (auto index : fill) {
         item.fill = {index};
         range = {pos, pos+size};
         items.append(item);
         pos += size;
      }
      return items;
   }
};

class Grid {
   int m_rows = {};
   int m_columns = {};
   QVector<Fill> m_data;
   friend QDebug operator<<(QDebug, const Grid &);
public:
   Grid() = default;
   Grid(int rows, int columns, const Fill & val = {}) :
      m_rows(rows), m_columns(columns), m_data(rows*columns, val) {}
   void add(const OpenRange & rows, const OpenRange & columns, const Fill & val) {
      for (auto r : rows)
         for (auto c : columns)
            at(r, c) = val;
   }
   void add(int row, int col, const Fill & val) {
      add({row, row+1}, {col, col+1}, val);
   }
   Fill & at(int row, int col) { return m_data[col*m_rows + row]; }
   const Fill & at(int row, int col) const { return m_data.at(col*m_rows + row); }
   GridItem itemAt(int row, int col) const {
      return {at(row, col), rowSpan(row, col), colSpan(row, col)};
   }
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
   bool anyEmpty() const {
      return anyEmpty(0, 0, m_rows, m_columns);
   }
   bool anyEmpty(int row, int col, int rowSpan, int colSpan) const {
      for (int r = row; r < row+rowSpan; ++r)
         for (int c = col; c < col+colSpan; ++c)
            if (at(r,c).begin() == at(r,c).end())
               return true;
      return false;
   }
   int rowCount() const { return m_rows; }
   int columnCount() const { return m_columns; }
   QVector<GridItem> toItems() const {
      QVector<GridItem> items;
      std::set<int> indexes;
      for (int r = 0; r < m_rows; ++r)
         for (int c = 0; c < m_columns; ++c) {
            auto const item = itemAt(r, c);
            if (item.fill.empty() || indexes.count(*item.fill.begin()))
               continue;
            for (auto i : item.fill) {
               Q_ASSERT(!indexes.count(i));
               indexes.insert(i);
            }
            items.append(item);
         }
      return items;
   }
   static Grid fromItems(const QVector<GridItem> & items) {
      int rows = 0, columns = 0;
      for (auto & item : items) {
         rows = std::max(rows, item.rows.right);
         columns = std::max(columns, item.columns.right);
      }
      Grid grid(rows, columns);
      for (auto & item : items)
         grid.add(item.rows, item.columns, item.fill);
      return grid;
   }
};

QDebug operator<<(QDebug dbg, const Grid & g) {
   QDebugStateSaver saver(dbg);
   dbg.nospace() << "Grid{";
   for (int r = 0; r < g.m_rows; ++r) {
      dbg << "[";
      for (int c = 0; c < g.m_columns; ++c) {
         auto const & cell = g.at(r, c);
         if (cell.empty())
            dbg << "-";
         else {
            if (cell.type == Fill::Type::Row)
               dbg << "r";
            else if (cell.type == Fill::Type::Column)
               dbg << "c";
            if (cell.size() == 1)
               dbg << *cell.begin();
            else {
               dbg << "(";
               unsigned int i = 0;
               for (auto v : cell) {
                  dbg << v;
                  if (i++ < cell.size() - 1) dbg << ",";
               }
               dbg << ")";
            }
         }
         if (c < g.m_columns-1) dbg << ",";
      }
      dbg << ((r < g.m_rows-1) ? "], " : "]");
   }
   return dbg << "}";
}

class FillGridLayout : public QGridLayout {
   Q_OBJECT
   struct Item {
      Item() = default;
      Item(const Item &) = default;
      Item(Item &&) = default;
      Item(QGridLayout * layout, int index, bool own) {
         int rowSpan, colSpan;
         layout->getItemPosition(index, &rows.left, &columns.left, &rowSpan, &colSpan);
         rows.right = rows.left + rowSpan;
         columns.right = columns.left + colSpan;
         item = layout->itemAt(index);
         if (own) layout->takeAt(index);
         //if (own) item.reset(layout->takeAt(index));
      }
      QLayoutItem * item;
      OpenRange rows, columns;
      bool isInvisible() const {
         return item && item->widget() && !item->widget()->isVisible();
      }
   };
   using Items = QVector<Item>;

   int m_rows, m_columns; // original layout
   Items m_items; // items from the original layout

   using Grids = QVector<Grid>;

   Items peek() {
      Items items;
      for (int i = 0; i < count(); ++i)
         items.append({this, i, false});
      return items;
   }

   Items take() {
      Items items;
      while (itemAt(0))
         items.append({this, 0, true});
      return items;
   }
   Grid getGrid() const {
      Grid grid(m_rows, m_columns);
      int i = 0;
      for (auto & item : m_items) {
         if (!item.isInvisible())
            grid.add(item.rows, item.columns, {i});
         i++;
      }
      return grid;
   }
   static Grids findAllGrids(const Grid & grid) {
      if (!grid.anyEmpty())
         return Grids(1, grid); // this is a solution
      static int level;
      Grids grids;
      ++level;
      for (int r = 0; r < grid.rowCount(); ++r)
         for (int c = 0; c < grid.columnCount(); ++c) {
            auto rowGrid = rowFillGrid(r, c, grid);
            auto colGrid = colFillGrid(r, c, grid);
            if (true && (!rowGrid.isEmpty() || !colGrid.isEmpty()))
               qDebug() << level << r << c << rowGrid << colGrid;
            if (!rowGrid.isEmpty())
               appendUnique(grids, findAllGrids(rowGrid));
            if (!colGrid.isEmpty())
               appendUnique(grids, findAllGrids(colGrid));
         }
      --level;
      return grids;
   }
   static Grid rowFillGrid(int row, int col, Grid grid) {
      auto item = grid.itemAt(row, col);
      if (item.empty())
         return {};
      auto const check = [row, &item, &grid](int col){
         if (col < 0 || col >= grid.columnCount())
            return false;
         auto next = grid.at(row, col);
         auto nextSpan = grid.rowSpan(row, col);
         return (next.empty() && nextSpan.contains(item.rows)) ||
               (!next.empty() && nextSpan == item.rows);
      };
      OpenRange cols(col, col+1);
      for (; check(cols.left-1); cols.left--);
      for (; check(cols.right); cols.right++);
      if (!grid.anyEmpty(row, cols.left, 1, cols.size()))
         return {};
      for (auto c : cols)
         insert(item.fill, grid.at(row, c));
      item.fill.type = Fill::Type::Row;
      grid.add(item.rows, cols, item.fill);
      return grid;
   }
   static Grid colFillGrid(int row, int col, Grid grid) {
      auto item = grid.itemAt(row, col);
      if (item.empty())
         return {};
      auto const check = [col, &item, &grid](int row){
         if (row < 0 || row >= grid.rowCount())
            return false;
         auto next = grid.at(row, col);
         auto nextSpan = grid.colSpan(row, col);
         return (next.empty() && nextSpan.contains(item.columns)) ||
               (!next.empty() && nextSpan == item.columns);
      };
      OpenRange rows(row, row+1);
      for (; check(rows.left-1); rows.left--);
      for (; check(rows.right); rows.right++);
      if (!grid.anyEmpty(rows.left, col, rows.size(), 1))
         return {};
      for (auto r : rows)
         insert(item.fill, grid.at(r, col));
      item.fill.type = Fill::Type::Column;
      grid.add(rows, item.columns, item.fill);
      return grid;
   }
   static Grid solution(const Grid & src) {
      QVector<GridItem> items;
      for (auto item : src.toItems())
         items.append(item.split());
      solve(items, Fill::Type::Row);
      solve(items, Fill::Type::Column);
      return Grid::fromItems(items);
   }
   static void solve(QVector<GridItem> & items, Fill::Type type) {
      std::set<int> denominators;
      for (auto const & item : items)
         denominators.insert(item.denominator);
      auto const denominator = LCM(denominators.begin(), denominators.end());
      std::set<int> coordinates;
      for (auto const & item : items) {
         Q_ASSERT(denominator % item.denominator == 0);
         auto range = item.range(type).scaled(denominator / item.denominator);
         coordinates.insert(range.left);
         coordinates.insert(range.right);
      }
      QVector<int> coordinateMap(*coordinates.rbegin() + 1, -1);
      int i = 0;
      for (auto coord : coordinates)
         coordinateMap[coord] = i++;
      for (auto & item : items) {
         auto & range = item.range(type);
         auto srcRange = range.scaled(denominator / item.denominator);
         range.left = coordinateMap[srcRange.left];
         range.right = coordinateMap[srcRange.right];
         Q_ASSERT(range.left >= 0 && range.right >= 0);
      }
   }
public:
   FillGridLayout(QWidget * parent = {}) : QGridLayout(parent) {}
   void convert() {
      m_rows = rowCount();
      m_columns = columnCount();
      m_items = peek();
      auto const grid = getGrid();
      auto const simpleGrid = solution(grid);
      qDebug() << "1." << grid << "->" << simpleGrid;
      auto grids = findAllGrids(simpleGrid);
      qDebug() << "2." << simpleGrid << "->" << grids;
      for (auto & grid : grids)
         grid = solution(grid);
      qDebug() << "3." << simpleGrid << "->" << grids;
   }
   void refill() {

   }
};

void clearWidgets(QLayout * layout) {
   while (auto item = layout->takeAt(0)) {
      delete item->widget();
      delete item;
   }
}

class TestUi : public QWidget {
   QGridLayout m_layout{this};
   QPlainTextEdit m_source;
   QGridLayout m_left;
   FillGridLayout m_right;
   QPushButton m_reset{"Reset"};
   QPushButton m_regen{"Regen"};
   int toIndex(QChar c) {
      if (c >= '0' && c <= '9') return c.unicode() - '0';
      if (c >= 'A' && c <= 'Z') return c.unicode() + 10 - 'A';
      return -1;
   }
   Grid gridFromText(const QString & str) {
      auto lines = str.toUpper().split('\n', QString::SkipEmptyParts);
      int rows = lines.size();
      int columns = 0;
      for (auto & line : lines) {
         while (line.endsWith(' ')) line.chop(1);
         columns = std::max(columns, line.size());
      }
      Grid grid(rows, columns);
      for (int r = 0; r < rows; ++r) {
         auto const & line = lines[r];
         for (int c = 0; c < line.size(); ++c) {
            int i = toIndex(line[c]);
            if (i >= 0)
               grid.add(r, c, {i});
         }
      }
      return grid;
   }
   void reset() {
      clearWidgets(&m_left);
      clearWidgets(&m_right);
      auto grid = gridFromText(m_source.toPlainText());
      std::set<int> added;
      for (int r = 0; r < grid.rowCount(); r++)
         for (int c = 0; c < grid.columnCount(); c++)
            if (!grid.anyEmpty(r, c, 1, 1)) {
               auto const item = grid.itemAt(r, c);
               if (!added.count(item.index())) {
                  add(item);
                  added.insert(item.index());
               }
            }
   }
   void add(const GridItem & i) {
      auto text = QString::number(i.index());
      auto button = new QPushButton{text};
      auto label = new QLabel{text};
      button->setCheckable(true);
      button->setChecked(true);
      label->setFrameStyle(QFrame::Box | QFrame::Plain);
      connect(button, &QPushButton::toggled, label, &QLabel::setVisible);
      m_left.addWidget(button, i.row(), i.column(), i.rowSpan(), i.colSpan());
      m_right.addWidget(label, i.row(), i.column(), i.rowSpan(), i.colSpan());
   }
public:
   TestUi() {
      m_layout.addWidget(&m_source, 0, 0);
      m_layout.addLayout(&m_left, 0, 1);
      m_layout.addLayout(&m_right, 0, 2);
      m_layout.addWidget(&m_reset, 1, 0);
      m_layout.addWidget(&m_regen, 1, 1);
      m_source.setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
      connect(&m_reset, &QPushButton::clicked, this, &TestUi::reset);
      connect(&m_regen, &QPushButton::clicked, [this]{ m_right.convert(); });
      auto format = m_source.currentCharFormat();
      auto font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
      font.setPointSize(13);
      format.setFont(font);
      m_source.setCurrentCharFormat(format);
      m_source.setPlainText("036\n147\n258");
      reset();
   }
};

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   TestUi ui;
   ui.show();
   return app.exec();
}
#include "main.moc"
