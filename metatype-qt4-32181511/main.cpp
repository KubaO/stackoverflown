#include <QtCore>

struct T {};

class C : public QObject {
   Q_OBJECT
   Q_PROPERTY(QList<T*> stuff READ stuff)
   QList<T*> m_stuff;
public:
   C(QObject * parent = 0) : QObject(parent) { m_stuff << (T*)10 << (T*)11; }
   Q_SIGNAL void aSignal(const QList<T*>&);
   Q_SLOT void aSlot(const QList<T*>& data) {
      m_stuff = data;
   }
   QList<T*> stuff() const {
      return m_stuff;
   }
};
Q_DECLARE_METATYPE(QList<T*>)

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);
   qRegisterMetaType<QList<T*> >();
   C c;
   c.connect(&c, SIGNAL(aSignal(QList<T*>)), SLOT(aSlot(QList<T*>)), Qt::QueuedConnection);

   // Test reading a property
   QMetaProperty p = c.metaObject()->property(c.metaObject()->indexOfProperty("stuff"));
   auto s = p.read(&c).value<QList<T*> >();
   Q_ASSERT(s == c.stuff());

   // Test queued signal-slot connection
   auto const data = QList<T*>() << (T*)1 << (T*)2;
   emit c.aSignal(data);
   a.processEvents();
   Q_ASSERT(data == c.stuff());
   return 0;
}

#include "main.moc"
