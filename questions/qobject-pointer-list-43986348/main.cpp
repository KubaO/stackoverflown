// https://github.com/KubaO/stackoverflown/tree/master/questions/qobject-pointer-list-43986348
#include <QtCore>

class PointerListData : public QObject, public QSharedData {
   Q_OBJECT
public:
   QVector<QObject*> list;
   void removed() { list.removeAll(sender()); }
   void connect(QObject* obj) {
      QObject::connect(obj, &QObject::destroyed, this, &PointerListData::removed);
   }
   void disconnect(QObject* obj) {
      QObject::disconnect(obj, &QObject::destroyed, this, &PointerListData::removed);
   }
};

template <typename T> class PointerList {
protected:
   QExplicitlySharedDataPointer<PointerListData> d;
public:
   PointerList() : d(new PointerListData) {}
   PointerList(const PointerList &other) : d(other.d) {}
   PointerList(PointerList && other) : d(std::move(other.d)) {}
   void append(T* obj) {
      auto connect = !contains(obj);
      d->list.append(obj);
      if (connect)
         d->connect(obj);
   }
   PointerList & operator<<(T* obj) {
      append(obj);
      return *this;
   }
   int removeAll(T* obj) {
      auto n = d->list.removeAll(obj);
      if (n)
         d->disconnect(obj);
      return n;
   }
   bool contains(T* obj) const {
      return d->list.contains(obj);
   }
   void clear() {
      for (auto obj : d->list)
         d->disconnect(obj);
      d->list.clear();
   }

   void moveToThread(QThread* thread) { d->moveToThread(thread); }
   bool isEmpty() const { return d->list.isEmpty(); }
   int size() const { return d->list.size(); }
   using iterator = T**;
   using const_iterator = const T**;
   iterator begin() { return iterator(d->list.data()); }
   iterator end() { return iterator(d->list.data() + d->list.size()); }
   const_iterator begin() const { return const_iterator(d->list.constData()); }
   const_iterator end() const { return const_iterator(d->list.constData() + d->list.size()); }
   constexpr const PointerList& crange() const noexcept { return *this; }
     // see http://stackoverflow.com/q/15518894/1329652
};

int main(int argc, char ** argv) {
   QCoreApplication app(argc, argv);
   PointerList<QMimeData> list;
   {
      QMimeData a;
      QMimeData b;
      list << &a << &b;
      auto list2 = list;
      Q_ASSERT(list2.size() == 2);
      for (auto obj : list.crange())
         qDebug() << obj;
   }
   Q_ASSERT(list.isEmpty());

}
#include "main.moc"
