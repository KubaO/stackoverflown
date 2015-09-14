// https://github.com/KubaO/stackoverflown/tree/master/questions/scene-iterator-adapter-32567814
#include <QGraphicsScene>
#include <QList>
#include <QDebug>
#include <utility>

template<class> class IterableSceneAdapter;

template<typename IT>
class ItemListIterator {
   const QList<QGraphicsItem*> * m_items;
   int m_index;
   friend class IterableSceneAdapter<IT>;
   ItemListIterator(const QList<QGraphicsItem*> * items, int dir) :
      m_items(items), m_index(dir>0 ? -1 : items->count()) {
      if (dir > 0) ++*this;
   }
   friend QDebug operator<<(QDebug dbg, const ItemListIterator & it) {
      return dbg << (it.m_items->isEmpty() ? nullptr : it.m_items->first()->scene())
                 << it.m_index;
   }
   friend void swap(ItemListIterator& a, ItemListIterator& b) {
      std::swap(a.m_items, b.m_items);
      std::swap(a.m_index, b.m_index);
   }
   template <typename T> T item_cast(QGraphicsItem* item) {
      // use qgraphicsitem_cast if you don't have RTTI enabled
      return dynamic_cast<T>(item);
   }
public:
   ItemListIterator() : m_index(0) {}
   ItemListIterator(const ItemListIterator & o) :
      m_items(o.m_items), m_index(o.m_index) {}
   ItemListIterator(ItemListIterator && o) { swap(*this, o); }
   ItemListIterator & operator=(ItemListIterator o) {
      swap(*this, o);
      return *this;
   }
   IT * operator*() const { return static_cast<IT*>(m_items->at(m_index)); }
   const ItemListIterator & operator++() {
      while (++m_index < m_items->count() && !item_cast<IT*>(m_items->at(m_index)));
      return *this;
   }
   ItemListIterator operator++(int) {
      ItemListIterator temp{*this};
      ++*this;
      return temp;
   }
   const ItemListIterator & operator--() {
      while (!item_cast<IT*>(m_items->at(--m_index)) && m_index > 0);
      return *this;
   }
   ItemListIterator operator--(int) {
      ItemListIterator temp(*this);
      --*this;
      return temp;
   }
   bool operator==(const ItemListIterator & o) const { return m_index == o.m_index; }
   bool operator!=(const ItemListIterator & o) const { return m_index != o.m_index; }
};

template <class IT = QGraphicsItem>
class IterableSceneAdapter {
   const QList<QGraphicsItem*> m_items;
public:
   typedef ItemListIterator<IT> iterator;
   typedef iterator const_iterator;
   IterableSceneAdapter(QGraphicsScene * scene) : m_items(scene->items()) {}
   const_iterator begin() const { return const_iterator(&m_items, 1); }
   const_iterator end() const { return const_iterator(&m_items, -1); }
   const_iterator cbegin() const { return const_iterator(&m_items, 1); }
   const_iterator cend() const { return const_iterator(&m_items, -1); }
};

template <class IT = QGraphicsItem>
class ConstIterableSceneAdapter : public IterableSceneAdapter<const IT> {
public:
   ConstIterableSceneAdapter(const QGraphicsScene * scene) :
      IterableSceneAdapter<const IT>(const_cast<QGraphicsScene*>(scene)) {}
};

#include <QtWidgets>

void tests() {
   QGraphicsScene s;
   QGraphicsLineItem b1, b3;
   QGraphicsEllipseItem b2;

   IterableSceneAdapter<> s0{&s};
   auto i0 = s0.begin();
   qDebug() << i0; Q_ASSERT(i0 == s0.begin() && i0 == s0.end());

   s.addItem(&b1);
   s.addItem(&b2);
   s.addItem(&b3);

   IterableSceneAdapter<> s1{&s};
   auto i1 = s1.begin();
         qDebug() << i1; Q_ASSERT(i1 == s1.begin() && i1 != s1.end());
   ++i1; qDebug() << i1; Q_ASSERT(i1 != s1.begin() && i1 != s1.end());
   ++i1; qDebug() << i1; Q_ASSERT(i1 != s1.begin() && i1 != s1.end());
   ++i1; qDebug() << i1; Q_ASSERT(i1 != s1.begin() && i1 == s1.end());
   --i1; qDebug() << i1; Q_ASSERT(i1 != s1.begin() && i1 != s1.end());
   --i1; qDebug() << i1; Q_ASSERT(i1 != s1.begin() && i1 != s1.end());
   --i1; qDebug() << i1; Q_ASSERT(i1 == s1.begin() && i1 != s1.end());

   IterableSceneAdapter<QGraphicsEllipseItem> s2{&s};
   auto i2 = s2.begin();
         qDebug() << i2; Q_ASSERT(i2 == s2.begin() && i2 != s2.end());
   ++i2; qDebug() << i2; Q_ASSERT(i2 != s2.begin() && i2 == s2.end());
   --i2; qDebug() << i2; Q_ASSERT(i2 == s2.begin() && i2 != s2.end());
}

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   tests();
   QGraphicsScene s;
   QGraphicsLineItem b1, b3;
   QGraphicsEllipseItem b2;

   s.addItem(&b1);
   s.addItem(&b2);
   s.addItem(&b3);

   // Iterate all item types as constant items
   qDebug() << "all, range-for";
   for (auto item : ConstIterableSceneAdapter<>(&s)) qDebug() << item;
   qDebug() << "all, Q_FOREACH";
   Q_FOREACH (const QGraphicsItem * item, ConstIterableSceneAdapter<>(&s)) qDebug() << item;

   // Iterate ellipses only
   qDebug() << "ellipses, range-for";
   for (auto item : IterableSceneAdapter<QGraphicsEllipseItem>(&s)) qDebug() << item;
   qDebug() << "ellipses, Q_FOREACH";
   Q_FOREACH (QGraphicsEllipseItem * item, IterableSceneAdapter<QGraphicsEllipseItem>(&s)) qDebug() << item;
}
