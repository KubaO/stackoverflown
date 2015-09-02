#include <QCoreApplication>
#include <QDataStream>
#include <QMetaProperty>
#include <QMetaObject>
#include <QStringList>
#include <QDebug>

class CustomObject : public QObject
{
   Q_OBJECT
   Q_PROPERTY(QString name READ name WRITE setName STORED true)
   QString m_name;
public:
   explicit CustomObject(QObject *parent = 0) : QObject(parent) {}
   CustomObject(const CustomObject & copy, QObject *parent = 0);

   QString name() const { return m_name; }
   void setName( const QString & name) { m_name = name; }
};

Q_DECLARE_METATYPE( CustomObject )

/// Returns a zero-copy byte array wrapping a C string constant
static QByteArray baFromCStr(const char * str)
{
   return QByteArray::fromRawData(str, qstrlen(str));
}

template <typename T>
QDataStream & writeObjectList(QDataStream& str, const QList<T*> & items)
{
   str << (quint32)items.count();
   if (! items.count()) return str;
   str << (quint8)0; // version
   const QMetaObject * mo = items[0]->metaObject();
   str << baFromCStr(mo->className());
   QList<QByteArray> stored;
   for (int i = 0; i < mo->propertyCount(); ++i)
      if (mo->property(i).isStored(items[0]))
         stored << baFromCStr(mo->property(i).name());
   str << stored;
   QMap<QByteArray, qint32> dynamic;
   foreach (QObject* item, items) {
      // Stored properties
      foreach (const QByteArray & name, stored)
         str << item->property(name);
      // Dynamic properties
      str << (qint32)item->dynamicPropertyNames().count();
      foreach (const QByteArray & name, item->dynamicPropertyNames()) {
         QMap<QByteArray, qint32>::const_iterator it = dynamic.find(name);
         if (it == dynamic.end()) {
            str << (qint32)dynamic.count() << name;
            dynamic.insert(name, dynamic.count());
         } else {
            str << it.value();
         }
         str << item->property(name);
      }
   }
   return str;
}

template <typename T> QDataStream & readObjectList(QDataStream& str,
                                                   QList<T*> & items)
{
   quint32 itemCount;
   str >> itemCount;
   if (!itemCount) return str;
   quint8 version;
   QByteArray className;
   QList<QByteArray> stored;
   str >> version >> className >> stored;
   Q_ASSERT(className == T::staticMetaObject.className());
   Q_ASSERT(version == 0);
   QList<QByteArray> dynamic;
   for (quint32 i = 0; i < itemCount; ++i) {
      QScopedPointer<T> item(new T());
      // Stored properties
      foreach (const QByteArray & name, stored) {
         QVariant var;
         str >> var;
         item->setProperty(name, var);
      }
      // Dynamic properties
      qint32 dCount;
      str >> dCount;
      for (int i = 0; i < dCount; ++i) {
         qint32 dIndex;
         str >> dIndex;
         QByteArray name;
         Q_ASSERT(dIndex <= dynamic.count());
         if (dIndex == dynamic.count()) {
            str >> name;
            dynamic << name;
         } else {
            name = dynamic[dIndex];
         }
         QVariant var;
         str >> var;
         item->setProperty(name, var);
      }
      items << item.take();
   }
   return str;
}

QDataStream& operator<<( QDataStream& str, const QList<CustomObject*> & items )
{
   return writeObjectList(str, items);
}

QDataStream& operator>>( QDataStream& str, QList<CustomObject*> & items )
{
   return readObjectList(str, items);
}

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);
   QObject o;
   o.setProperty("foo", 10);
   const QMetaObject * mo = o.metaObject();
   for (int i = 0; i < mo->propertyCount(); ++i) {
      qDebug() << mo->property(i).name();
   }
   qDebug() << o.dynamicPropertyNames();

   return a.exec();
}

#include "main.moc"
