#include <QList>
#include <QString>
#include <functional>

template <typename T>
class RefList : public QList<std::reference_wrapper<typename std::add_const<T>::type>> {
   using base = QList<std::reference_wrapper<typename std::add_const<T>::type>>;
   using val_type = typename std::remove_const<T>::type;
   using nc_base = QList<std::reference_wrapper<val_type>>;
   using list_type = QList<val_type>;

  public:
   struct iterator : base::iterator {
      using base_it = typename base::iterator;
      using base_it::iterator::iterator;
      iterator(const base_it &o) : base_it(o) {}
      val_type &operator*() const {
         return const_cast<val_type &>(
             static_cast<const QString &>(base_it::operator*()));
      }
   };
   // using iterator = typename QList<std::reference_wrapper<val_type>>::iterator;
   using base::base;
   RefList(const list_type &o) {
      this->reserve(o.size());
      for (auto &item : o) this->push_back(item);
   }
   RefList(const QList<val_type *> &o) {
      this->reserve(o.size());
      for (auto *item : o) this->push_back(*item);
   }
   RefList(const QList<const val_type *> &o) {
      this->reserve(o.size());
      for (auto *item : o) this->push_back(*item);
   }
   RefList &operator<<(const T &item) {
      return static_cast<RefList &>(base::operator<<(item));
   }
   RefList &operator<<(const RefList &list) {
      return static_cast<RefList &>(base::operator<<(list));
   }
   RefList &operator<<(list_type &list) {
      append(list);
      return *this;
   }
   using base::append;
   void append(const list_type &list) {
      this->reserve(this->size() + list.size());
      for (auto &item : list) this->push_back(item);
   }
   // using base::at;
   // using base::back;

   using base::begin;
   using base::end;
   iterator begin() { return base::begin(); }
   iterator end() { return base::end(); }

   static RefList<T> fromSet(const QSet<val_type> &) = delete;
   static RefList<T> fromStdList(const std::list<val_type> &) = delete;
   static RefList<T> fromVector(const QVector<val_type> &) = delete;
};

template <typename T>
QDataStream &operator>>(QDataStream &, RefList<T> &) = delete;

#include <QtTest>

class RefListTest : public QObject {
   Q_OBJECT
   QString a{"foo"};
   QString b{"bar"};
   QList<QString *> list_p_ab{&a, &b};
   QList<const QString *> list_cp_ab{&a, &b};
   QStringList list_ab{a, b};
   QStringList list_ba{b, a};
   QStringList list_abba{a, b, b, a};
   QStringList list_abba2{a, b, b, a, a, b, b, a};
   QStringList list_ab2{a, b, a, b};
   QStringList list_ab4{a, b, a, b, a, b, a, b};
   template <typename T, typename U>
   static bool checkList(const T &cmp, const U &gold) {
      if (cmp.size() != gold.size()) return false;
      for (int i = 0; i < cmp.size(); ++i)
         if (cmp[i] != gold[i]) return false;
      return true;
   }
   template <typename U, typename V>
   using is_same_dec = typename std::is_same<typename std::decay<U>::type, V>;
   Q_SLOT void defaultConstructible() { RefList<QString> rList; }
   Q_SLOT void acceptsT() {
      RefList<QString> rList;
      auto &ref = rList << a << qAsConst(b);
      rList.append(b);
      rList.append(qAsConst(a));
      QVERIFY(checkList(rList, list_abba));
      QVERIFY((is_same_dec<decltype(ref), decltype(rList)>::value));
   }
   Q_SLOT void copyConstructible() {
      RefList<QString> rList;
      rList << a << b;
      RefList<QString> rList2(rList);
      QVERIFY(checkList(rList2, list_ab));
   }
   Q_SLOT void moveConstructible() {
      RefList<QString> rList(RefList<QString>() << a << b);
      QVERIFY(checkList(rList, list_ab));
   }
   Q_SLOT void constructibleFromLists() {
      RefList<QString> rList(list_ab);
      QVERIFY(checkList(rList, list_ab));
      RefList<QString> rList2(qAsConst(list_ab));
      QVERIFY(checkList(rList2, list_ab));
   }
   Q_SLOT void constructibleFromPtrLists() {
      RefList<QString> rList(list_p_ab);
      QVERIFY(checkList(rList, list_ab));
      RefList<QString> rList2(qAsConst(list_p_ab));
      QVERIFY(checkList(rList2, list_ab));
   }
   Q_SLOT void constructibleFromConstPtrLists() {
      RefList<QString> rList(list_cp_ab);
      QVERIFY(checkList(rList, list_ab));
      RefList<QString> rList2(qAsConst(list_cp_ab));
      QVERIFY(checkList(rList2, list_ab));
   }

   Q_SLOT void acceptsLists() {
      RefList<QString> rList;
      auto &ref = rList << list_ab << qAsConst(list_ab);
      rList.append(list_ab);
      rList.append(qAsConst(list_ab));
      QVERIFY(checkList(rList, list_ab4));
      QVERIFY((is_same_dec<decltype(ref), decltype(rList)>::value));
   }
   Q_SLOT void acceptsRLists() {
      RefList<QString> rList, gList;
      gList << list_ab;
      auto &ref = rList << gList << qAsConst(gList);
      QVERIFY(checkList(rList, list_ab2));
      QVERIFY((is_same_dec<decltype(ref), decltype(rList)>::value));
   }
   Q_SLOT void hasAt() {
      RefList<QString> rList(list_ab);
      QCOMPARE(rList.at(0), a);
      QCOMPARE(qAsConst(rList).at(1), b);
   }
   Q_SLOT void hasBack() {
      RefList<QString> rList(list_ab);
      QCOMPARE(rList.back(), b);
      QCOMPARE(qAsConst(rList).back(), b);
      rList.back() = a;
      QVERIFY(checkList(rList, QStringList{a, a}));
   }
   Q_SLOT void hasBegin() {
      RefList<QString> rList(list_ab);
      QCOMPARE(*rList.begin(), a);
      QCOMPARE(*qAsConst(rList).begin(), a);
      QCOMPARE(*rList.cbegin(), a);
      QCOMPARE(*rList.constBegin(), a);
      *rList.begin() = b;
      QVERIFY(checkList(rList, QStringList{b, b}));
      QVERIFY((std::is_convertible<decltype(*rList.begin()), QString &>::value));
      QVERIFY(
          (!std::is_convertible<decltype(*qAsConst(rList).begin()), QString &>::value));
      QVERIFY((std::is_convertible<decltype(*qAsConst(rList).begin()),
                                   const QString &>::value));
      QVERIFY((!std::is_convertible<decltype(*rList.cbegin()), QString &>::value));
      QVERIFY((std::is_convertible<decltype(*rList.cbegin()), const QString &>::value));
      QVERIFY((!std::is_convertible<decltype(*rList.constBegin()), QString &>::value));
      QVERIFY(
          (std::is_convertible<decltype(*rList.constBegin()), const QString &>::value));
   }
   Q_SLOT void hasEnd() {
      RefList<QString> rList(list_ab);
      QCOMPARE(*std::prev(rList.end()), b);
      QCOMPARE(*std::prev(qAsConst(rList).end()), b);
      QCOMPARE(*std::prev(rList.cend()), b);
      QCOMPARE(*std::prev(rList.constEnd()), b);
      *std::prev(rList.end()) = a;
      QVERIFY(checkList(rList, QStringList{a, a}));
   }
   Q_SLOT void hasClear() {
      RefList<QString> rList(list_ab);
      rList.clear();
      QVERIFY(rList.isEmpty());
   }

   Q_SLOT void streamsLikeQList() {
      RefList<QString> rList;
      rList << a << b;
      QByteArray d1, d2;
      QDataStream ds1(&d1, QIODevice::WriteOnly);
      ds1 << rList;
      QDataStream ds2(&d2, QIODevice::WriteOnly);
      ds2 << list_ab;
      QVERIFY(!d1.isEmpty());
      QCOMPARE(d1, d2);
   }
   Q_SLOT void constStreamsLikeQList() {
      RefList<const QString> rList;
      rList << a << b;
      QByteArray d1, d2;
      QDataStream ds1(&d1, QIODevice::WriteOnly);
      ds1 << rList;
      QDataStream ds2(&d2, QIODevice::WriteOnly);
      ds2 << list_ab;
      QVERIFY(!d1.isEmpty());
      QCOMPARE(d1, d2);
   }
   Q_SLOT void debugsLikeQList() {
      RefList<QString> rList;
      rList << a << b;
      QString s1, s2;
      QDebug(&s1) << rList;
      QDebug(&s2) << list_ab;
      QVERIFY(!s1.isEmpty());
      QCOMPARE(s1, s2);
   }
   Q_SLOT void constDebugsLikeQList() {
      RefList<const QString> rList;
      rList << a << b;
      QString s1, s2;
      QDebug(&s1) << rList;
      QDebug(&s2) << list_ab;
      QVERIFY(!s1.isEmpty());
      QCOMPARE(s1, s2);
   }
};

QTEST_MAIN(RefListTest)
#include "main.moc"
