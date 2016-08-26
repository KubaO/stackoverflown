// https://github.com/KubaO/stackoverflown/tree/master/questions/layout-iterator-adapter-31546511
#include <QLayout>
#include <QDebug>
#include <QPointer>
#include <utility>

template<class> class IterableLayoutAdapter;

template<typename WT>
class LayoutIterator {
   QPointer<QLayout> m_layout;
   int m_index;
   friend class IterableLayoutAdapter<WT>;
   LayoutIterator(QLayout * layout, int dir) :
      m_layout(layout), m_index(dir>0 ? -1 : m_layout->count()) {
      if (dir > 0) ++*this;
   }
   friend QDebug operator<<(QDebug dbg, const LayoutIterator & it) {
      return dbg << it.m_layout << it.m_index;
   }
   friend void swap(LayoutIterator& a, LayoutIterator& b) {
      using std::swap;
      swap(a.m_layout, b.m_layout);
      swap(a.m_index, b.m_index);
   }
public:
   LayoutIterator() : m_index(0) {}
   LayoutIterator(const LayoutIterator & o) :
      m_layout(o.m_layout), m_index(o.m_index) {}
   LayoutIterator(LayoutIterator && o) { swap(*this, o); }
   LayoutIterator & operator=(LayoutIterator o) {
      swap(*this, o);
      return *this;
   }
   WT * operator*() const { return static_cast<WT*>(m_layout->itemAt(m_index)->widget()); }
   const LayoutIterator & operator++() {
      while (++m_index < m_layout->count() && !qobject_cast<WT*>(m_layout->itemAt(m_index)->widget()));
      return *this;
   }
   LayoutIterator operator++(int) {
      LayoutIterator temp(*this);
      ++*this;
      return temp;
   }
   const LayoutIterator & operator--() {
      while (!qobject_cast<WT*>(m_layout->itemAt(--m_index)->widget()) && m_index > 0);
      return *this;
   }
   LayoutIterator operator--(int) {
      LayoutIterator temp(*this);
      --*this;
      return temp;
   }
   bool operator==(const LayoutIterator & o) const { return m_index == o.m_index; }
   bool operator!=(const LayoutIterator & o) const { return m_index != o.m_index; }
};

template <class WT = QWidget>
class IterableLayoutAdapter {
   QPointer<QLayout> m_layout;
public:
   typedef LayoutIterator<WT> iterator;
   typedef iterator const_iterator;
   IterableLayoutAdapter(QLayout * layout) : m_layout(layout) {}
   const_iterator begin() const { return const_iterator(m_layout, 1); }
   const_iterator end() const { return const_iterator(m_layout, -1); }
   const_iterator cbegin() const { return const_iterator(m_layout, 1); }
   const_iterator cend() const { return const_iterator(m_layout, -1); }
};

template <class WT = QWidget>
class ConstIterableLayoutAdapter : public IterableLayoutAdapter<const WT> {
public:
   ConstIterableLayoutAdapter(QLayout * layout) : IterableLayoutAdapter<const WT>(layout) {}
};

#include <QApplication>
#include <QLabel>
#include <QHBoxLayout>

void tests() {
   QWidget a, b1, b3;
   QLabel b2;
   QHBoxLayout l(&a);

   IterableLayoutAdapter<> l0(&l);
   auto i0 = l0.begin();
   qDebug() << i0; Q_ASSERT(i0 == l0.begin() && i0 == l0.end());

   l.addWidget(&b1);
   l.addWidget(&b2);
   l.addWidget(&b3);

   IterableLayoutAdapter<> l1(&l);
   auto i1 = l1.begin();
         qDebug() << i1; Q_ASSERT(i1 == l1.begin() && i1 != l1.end());
   ++i1; qDebug() << i1; Q_ASSERT(i1 != l1.begin() && i1 != l1.end());
   ++i1; qDebug() << i1; Q_ASSERT(i1 != l1.begin() && i1 != l1.end());
   ++i1; qDebug() << i1; Q_ASSERT(i1 != l1.begin() && i1 == l1.end());
   --i1; qDebug() << i1; Q_ASSERT(i1 != l1.begin() && i1 != l1.end());
   --i1; qDebug() << i1; Q_ASSERT(i1 != l1.begin() && i1 != l1.end());
   --i1; qDebug() << i1; Q_ASSERT(i1 == l1.begin() && i1 != l1.end());

   IterableLayoutAdapter<QLabel> l2(&l);
   auto i2 = l2.begin();
         qDebug() << i2; Q_ASSERT(i2 == l2.begin() && i2 != l2.end());
   ++i2; qDebug() << i2; Q_ASSERT(i2 != l2.begin() && i2 == l2.end());
   --i2; qDebug() << i2; Q_ASSERT(i2 == l2.begin() && i2 != l2.end());
}

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   tests();
   QWidget a, b1, b3;
   QLabel b2;
   QHBoxLayout l(&a);
   l.addWidget(&b1);
   l.addWidget(&b2);
   l.addWidget(&b3);

   // Iterate all widget types as constants
   qDebug() << "all, range-for";
   for (auto widget : ConstIterableLayoutAdapter<>(&l)) qDebug() << widget;
   qDebug() << "all, Q_FOREACH";
   Q_FOREACH (const QWidget * widget, ConstIterableLayoutAdapter<>(&l)) qDebug() << widget;

   // Iterate labels only
   qDebug() << "labels, range-for";
   for (auto label : IterableLayoutAdapter<QLabel>(&l)) qDebug() << label;
   qDebug() << "labels, Q_FOREACH";
   Q_FOREACH (QLabel * label, IterableLayoutAdapter<QLabel>(&l)) qDebug() << label;
}
