#include <QList>
#include <QString>
#include <functional>

template <typename T> class RefList : public QList<std::reference_wrapper<T>> {
   using base = QList<std::reference_wrapper<T>>;
   using val_type = typename std::remove_const<T>::type;
   using list_type = typename std::conditional<std::is_const<T>::value,
                     const QList<val_type>, QList<val_type>>::type;
public:
   using base::base;

   RefList &operator<<(T &item) {
      this->push_back(item);
      return *this;
   }
   RefList &operator<<(list_type &list) {
      this->reserve(this->size() + list.size());
      for (auto &s : list)
         this->push_back(s);
      return *this;
   }
};

template <typename T> QDataStream &operator>>(QDataStream &, RefList<T> &) = delete;

#include <QtTest>

class RefListTest : public QObject {
   Q_OBJECT

   Q_SLOT void main() {
      QString a("foo"), b("bar");
      QStringList sList;
      RefList<QString> rList;
      sList << a << b;
      rList << a << b;
      rList << sList;

      QByteArray d1, d2;
      QDataStream ds1(&d1, QIODevice::WriteOnly);
      ds1 << sList;
      QDataStream ds2(&d2, QIODevice::WriteOnly);
      ds2 << rList;
      Q_ASSERT(d1 == d2);

      QString s1, s2;
      QDebug(&s1) << sList;
      QDebug(&s2) << rList;
      Q_ASSERT(!s1.isEmpty());
      Q_ASSERT(s1 == s2);
   }
};

QTEST_MAIN(RefListTest)
#include "main.moc"
