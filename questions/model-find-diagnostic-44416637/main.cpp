// https://github.com/KubaO/stackoverflown/tree/master/questions/model-find-diagnostic-44416637
#include <QtGui>

class IndexSet : public QMap<QModelIndex, bool> {
public:
   void insert(const QModelIndex & key) { QMap::insert(key, true); }
};

struct FindState {
   QModelIndex index;
   int rows;
   int cols;
   int i = 0;
   int j = 0;
   FindState() = default;
   FindState(const QAbstractItemModel* model, const QModelIndex & index) :
      index(index),
      rows(model->rowCount(index)),
      cols(model->columnCount(index)) {}
   bool isInitial() const { return i == 0 && j == 0; }
   bool equalTo(const QVariant & value) const {
      return index.isValid() && index.data() == value;
   }
   bool operator==(const FindState & o) const {
      return index == o.index;
   }
};

QDebug operator<<(QDebug dbg, const FindState & s) {
   return dbg << "{" << s.index << "," << s.i << "," << s.j << "}";
}

QModelIndex findIndexByValue(const QAbstractItemModel* model, const QVariant& needle,
                             const QModelIndex& parent = {}) {
   int iterations = {};
   int maxDepth = {};
   const auto depthLimit = 100;
   IndexSet indexes;
   QStack<FindState> states;
   states.push({model, parent});
   for (; !states.isEmpty(); iterations++) {
      auto valid = true;
      auto & top = states.top();
      if (top.isInitial()) {
         if (states.size() > 1) {
            if (!top.index.isValid()) {
               qWarning() << "the index isn't valid, stack:" << states;
               valid = false;
            }
            if (!model->hasIndex(top.index.row(), top.index.column(), top.index.parent())) {
               qWarning() << "the index doesn't exist, stack:" << states;
               valid = false;
            }
         }
         if (indexes.contains(top.index)) {
            qWarning() << "skipping already seen index" << top.index;
            valid = false;
         }
         if (valid) {
            indexes.insert(top.index);
            if (top.equalTo(needle))
               break;
         }
      }
      if (valid && model->hasChildren(top.index) && top.i < top.rows && top.j < top.cols) {
         FindState state(model, model->index(top.i, top.j, top.index));
         top.i ++;
         if (top.i == top.rows) {
            top.i = 0;
            top.j ++;
         }
         if (states.contains(state))
            qWarning() << "skipping duplicate index" << state.index;
         else if (states.size() == depthLimit)
            qWarning() << "stack is full, skipping index" << state.index;
         else {
            states.push(state);
            maxDepth = std::max(maxDepth, states.size());
         }
      } else
         states.pop();
   }
   qDebug() << "max depth" << maxDepth << "iterations" << iterations;
   return states.isEmpty() ? QModelIndex() : states.top().index;
}

QModelIndex findIndexByString(const QAbstractItemModel* model, const QString& needle, const QModelIndex& parent = {}) {
   return findIndexByValue(model, QVariant::fromValue(needle), parent);
}

//

QModelIndex findIndexByString2(const QAbstractItemModel* model, const QString& text,  const QModelIndex& parent = QModelIndex());
QModelIndex findIndexByString2(const QModelIndex& index, const QString& text) {
   if (index.data().toString() == text)
      return index;
   auto model = index.model();
   if (!model || !model->hasChildren(index))
      return {};
   return findIndexByString2(model, text, index);
}

QModelIndex findIndexByString2(const QAbstractItemModel* model, const QString& text,  const QModelIndex& parent) {
   int rows = model->rowCount(parent);
   int cols = model->columnCount(parent);
   for (int i = 0; i < rows; ++i)
      for (int j = 0; j < cols; ++j) {
         auto child = findIndexByString2(model->index(i, j, parent), text);
         if (child.isValid())
            return child;
      }
   return {};
}

//

struct Pos {
   int row, col;
   QString toString() const { return QStringLiteral("(%1,%2)").arg(row).arg(col); }
};
Q_DECLARE_TYPEINFO(Pos, Q_PRIMITIVE_TYPE);
using Path = QVector<Pos>;

struct Builder {
   QStandardItemModel * model;
   QStringList paths;
   Path add(Path path, const Pos & pos) {
      auto item = model->invisibleRootItem();
      path.push_back(pos);
      QString pathString;
      for (auto p : path) {
         pathString += p.toString();
         auto child = item->child(p.row, p.col);
         if (!child) {
            item->setChild(p.row, p.col, child = new QStandardItem(pathString));
            paths << pathString;
         }
         item = child;
      }
      return path;
   }
   explicit Builder(QStandardItemModel * model) : model(model) {}
};

int main() {
   QStandardItemModel model;
   Builder b(&model);
   Path path, ____;
   path = b.add({}, {1, 0});     // *(1,0)
   ____ = b.add(path, {1, 1});   //  (1,0)-(1,1)
   ____ = b.add(path, {0, 0});   //  (1,0)-(0,0)
   path = b.add({}, {1, 1});     // *(1,1)
   path = b.add(path, {3, 3});   // *(1,1)-(3,3)
   ____ = b.add(path, {0, 0});   //  (1,1)-(3,3)-(0,0)
   path.pop_back();              // -(1,1)
   path = b.add(path, {2, 2});   // *(1,1)-(2,2)
   ____ = b.add(path, {0, 1});   //  (1,1)-(2,2)-(0,1)
   path = b.add({}, {2, 1});     // *(2,1)

   IndexSet indexes;
   for (auto path : b.paths) {
      auto index = findIndexByString(b.model, path);
      auto index2 = findIndexByString2(b.model, path);
      Q_ASSERT(index.isValid());
      Q_ASSERT(!indexes.contains(index));
      Q_ASSERT(index2 == index);
      indexes.insert(index);
   }
}
