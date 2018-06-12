// https://github.com/KubaO/stackoverflown/tree/master/questions/prop-storage-24185694
#include <QtCore>

class CustomObject : public QObject {
   Q_OBJECT
   Q_PROPERTY(QString name READ name WRITE setName STORED true)
   QString m_name;
public:
#ifdef Q_MOC_RUN
   Q_INVOKABLE CustomObject(QObject *parent = {})
#endif
   using QObject::QObject;

   QString name() const { return m_name; }
   void setName(const QString &name) { m_name = name; }
};

/// Returns a zero-copy byte array wrapping a C string constant
static QByteArray baFromCStr(const char *str) {
   return QByteArray::fromRawData(str, qstrlen(str));
}

/// Returns a list of stored properties for a given type
QList<QMetaProperty> storedProperties(const QMetaObject *mo) {
   QList<QMetaProperty> stored;
   for (int i = 0; i < mo->propertyCount(); ++i) {
      auto prop = mo->property(i);
      if (prop.isStored())
         stored << prop;
   }
   return stored;
}

/// Caches strings for saving to a data stream
struct SaveCache {
   QMap<QByteArray, qint32> strings;
   QDataStream &save(QDataStream &str, const QByteArray &string) {
      auto it = strings.find(string);
      if (it != strings.end())
         return str << (qint32)it.value();
      auto key = strings.count();
      strings.insert(string, key);
      return str << (qint32)key << string;
   }
   QDataStream &save(QDataStream &str, const char *string) {
      return save(str, baFromCStr(string));
   }
};

/// Caches strings while loading from a data stream
struct LoadCache {
   QList<QByteArray> strings;
   QDataStream &load(QDataStream &str, QByteArray &string) {
      qint32 index;
      str >> index;
      if (index >= strings.count()) {
         str >> string;
         while (strings.size() < index)
            strings << QByteArray{};
         strings << string;
      } else
         string = strings.at(index);
      return str;
   }
};

template <typename T>
QDataStream &writeObjectList(QDataStream &str, const QList<T*> &items) {
   str << (quint32)items.count();
   if (! items.count()) return str;
   str << (quint8)1; // version

   SaveCache strings;
   for (QObject *item : items) {
      auto *mo = item->metaObject();
      // Type
      strings.save(str, mo->className());
      // Properties
      auto const stored = storedProperties(mo);
      auto const dynamic = item->dynamicPropertyNames();
      str << (quint32)(stored.count() + dynamic.count());
      for (auto &prop : qAsConst(stored))
         strings.save(str, prop.name()) << prop.read(item);
      for (auto &name : dynamic)
         strings.save(str, name) << item->property(name);
   }
   return str;
}

template <typename T> QDataStream &readObjectList(QDataStream &str,
                                                  QList<T*> &items)
{
   quint32 itemCount;
   str >> itemCount;
   if (!itemCount) return str;
   quint8 version;
   str >> version;
   if (version != 1) {
      str.setStatus(QDataStream::ReadCorruptData);
      return str;
   }

   LoadCache strings;
   for (; itemCount; itemCount--) {
      QByteArray string;
      // Type
      T *obj = {};
      strings.load(str, string);
      if (T::staticMetaObject.className() == string)
         obj = new T();
      else {
         string.append('*');
         auto id = QMetaType::type(string);
         const auto *mo = QMetaType::metaObjectForType(id);
         if (mo)
            obj = qobject_cast<T*>(mo->newInstance());
      }
      if (obj)
         items << obj;
      // Properties
      quint32 propertyCount;
      str >> propertyCount;
      for (uint i = 0; i < propertyCount; ++i) {
         QVariant value;
         strings.load(str, string) >> value;
         if (obj) obj->setProperty(string, value);
      }
   }
   return str;
}

QDataStream &operator<<(QDataStream &str, const QList<CustomObject*> &items) {
   return writeObjectList(str, items);
}

QDataStream& operator>>(QDataStream &str, QList<CustomObject*> &items) {
   return readObjectList(str, items);
}

int main() {
   qRegisterMetaType<CustomObject*>(); // necessary for any classes derived from CustomObject*

   QBuffer buf;
   buf.open(QBuffer::ReadWrite);
   QDataStream ds(&buf);

   CustomObject obj;
   obj.setObjectName("customsky");
   obj.setProperty("prop", 20);
   QList<CustomObject*> list;
   list << &obj;
   ds << list;

   QList<CustomObject*> list2;
   buf.seek(0);
   ds >> list2;
   Q_ASSERT(list2.size() == list.size());
   for (int i = 0; i < list.size(); ++i) {
      auto *obj1 = list.at(i);
      auto *obj2 = list2.at(i);
      Q_ASSERT(obj1->objectName() == obj2->objectName());
      Q_ASSERT(obj1->dynamicPropertyNames() == obj2->dynamicPropertyNames());
      for (auto &name : obj1->dynamicPropertyNames())
         Q_ASSERT(obj1->property(name) == obj2->property(name));
   }
}

#include "main.moc"
