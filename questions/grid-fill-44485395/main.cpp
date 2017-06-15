// https://github.com/KubaO/stackoverflown/tree/master/questions/grid-fill-44485395
#include <QtWidgets>
#include <algorithm>
#include <cstdlib>
#include <set>

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
};

OpenRange operator&(OpenRange l, const OpenRange & r) { return l &= r; }

using Fill = std::set<int>;

void insert(Fill & dst, const Fill & src) {
   dst.insert(src.begin(), src.end());
}

struct GridItem {
   Fill fill;
   int row, col, rowSpan, colSpan;
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
   void add(int row, int col, int rowSpan, int colSpan, const Fill & val) {
      for (int r = row; r < row + rowSpan; ++r)
         for (int c = col; c < col + colSpan; ++c)
            at(r, c) = val;
   }
   Fill & at(int row, int col) { return m_data[col*m_rows + row]; }
   const Fill & at(int row, int col) const { return m_data.at(col*m_rows + row); }
   Fill get(int row, int col) const { return at(row, col); }
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
   QVector<GridItem> getItems() const {
      QVector<GridItem> items;
      std::set<int> indexes;
      for (int r = 0; r < m_rows; ++r)
         for (int c = 0; c < m_columns; ++c) {
            auto const fill = at(r, c);
            if (fill.empty() || indexes.count(*fill.begin()))
               continue;
            for (auto i : fill) {
               Q_ASSERT(!indexes.count(i));
               indexes.insert(i);
            }
            items.append({fill, r, c, rowSpan(r,c).size(), colSpan(r,c).size()});
         }
      return items;
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
         else if (cell.size() == 1)
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
      Item(Item &&) = defualt;
      Item(QLayout * layout, int index) {
         layout->getItemPosition(index, &row, &col, &rowSpan, &colSpan);
         item.reset(layout->takeAt(index));
      }
      QSharedPointer<QLayoutItem> item;
      int row, col, rowSpan, colSpan;
      bool isInvisible() const {
         return item && item->widget() && !item->widget()->isVisible();
      }
   };
   using Items = QVector<Item>;

   int m_rows, m_columns; // original layout
   Items m_items; // items from the original layout

   using Grids = QVector<Grid>;

   Items take() {
      Items items;
      while (itemAt(0))
         items.append({this, 0});
      return items;
   }
   Grid getGrid() const {
      Grid grid(m_rows, m_columns);
      int i = 0;
      for (auto & item : m_items) {
         if (!item.isInvisible())
            grid.add(item.row, item.col, item.rowSpan, item.colSpan, {i});
         i++;
      }
      return grid;
   }
   Grids findAllGrids(const Grid & grid) {
      if (!grid.anyEmpty())
         return Grids(1, grid);
      static int level;
      Grids grids;
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
   Grid rowFillGrid(int row, int col, Grid grid) const {
      auto fill = grid.at(row, col);
      if (fill.empty())
         return {};
      auto const rowSpan = grid.rowSpan(row, col);
      auto const check = [this, row, rowSpan, &grid](int col){
         if (col < 0 || col >= m_columns)
            return false;
         auto next = grid.at(row, col);
         auto nextSpan = grid.rowSpan(row, col);
         return (next.empty() && nextSpan.contains(rowSpan)) ||
               (!next.empty() && nextSpan == rowSpan);
      };
      int left = col, right = col+1;
      for (; check(left-1); left--);
      for (; check(right); right++);
      if (!grid.anyEmpty(row, left, 1, right-left))
         return {};
      for (int c = left; c < right; c++)
         insert(fill, grid.at(row, c));
      grid.add(rowSpan.left, left, rowSpan.size(), right-left, fill);
      return grid;
   }
   Grid colFillGrid(int row, int col, Grid grid) const {
      auto fill = grid.at(row, col);
      if (fill.empty())
         return {};
      auto const colSpan = grid.colSpan(row, col);
      auto const check = [this, col, colSpan, &grid](int row){
         if (row < 0 || row >= m_rows)
            return false;
         auto next = grid.at(row, col);
         auto nextSpan = grid.colSpan(row, col);
         return (next.empty() && nextSpan.contains(colSpan)) ||
               (!next.empty() && nextSpan == colSpan);
      };
      int top = row, bottom = row+1;
      for (; check(top-1); top--);
      for (; check(bottom); bottom++);
      if (!grid.anyEmpty(top, col, bottom-top, 1))
         return {};
      for (int r = top; r < bottom; r++)
         insert(fill, grid.at(r, col));
      grid.add(top, colSpan.left, bottom-top, colSpan.size(), fill);
      return grid;
   }
   Grid solution(const Grid & src) {
      struct {
         Fill items;
         int row, col, rowSpan, ColSpan;
      };
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
      m_items = take();
      auto grid = getGrid();
      qDebug() << grid;
      qDebug() << grid << "->" << findAllGrids(grid);
   }
   void refill() {

   }
   QLayoutItem * takeAt(int index) override {
      auto item =
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
               grid.add(r, c, 1, 1, {i});
         }
      }
      return grid;
   }
   void reset() {
      clearWidgets(&m_left);
      clearWidgets(&m_right);
      auto grid = gridFromText(m_source.toPlainText());
      Fill added;
      for (int r = 0; r < grid.rowCount(); r++)
         for (int c = 0; c < grid.columnCount(); c++)
            if (!grid.anyEmpty(r, c, 1, 1)) {
               auto index = *grid.at(r, c).begin();
               if (!added.count(index)) {
                  auto rows = grid.rowSpan(r, c);
                  auto cols = grid.colSpan(r, c);
                  add(index, rows.left, cols.left, rows.size(), cols.size());
                  added.insert(index);
               }
            }
   }
   void add(int index, int r, int c, int rowSpan, int colSpan) {
      auto text = QString::number(index);
      auto button = new QPushButton{text};
      auto label = new QLabel{text};
      button->setCheckable(true);
      button->setChecked(true);
      label->setFrameStyle(QFrame::Box | QFrame::Plain);
      connect(button, &QPushButton::toggled, label, &QLabel::setVisible);
      m_left.addWidget(button, r, c, rowSpan, colSpan);
      m_right.addWidget(label, r, c, rowSpan, colSpan);
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
