//main.cpp
#include <QDebug>

class Data {
   int m_i;
public:
   Data(const Data & o) : m_i(o.m_i) {
      qDebug() << "copy constructed";
   }
   Data(int i = 0) : m_i(i) {
      qDebug() << "constructed";
   }
   Data(Data && o) : m_i(o.m_i) {
      qDebug() << "move constructed";
   }
   ~Data() {
      qDebug() << "destructed";
   }
   int i() const { return m_i; }
};
Q_DECLARE_METATYPE(Data)

QDebug operator<<(QDebug dbg, const Data & d)
{
   dbg << "Data:" << d.i();
   return dbg;
}

class Object : public QObject {
   Q_OBJECT
public:
   Q_SIGNAL void source(const Data &);
   Q_SLOT void sink(const Data & d) {
      qDebug() << "sinking" << d;
   }
};

int main()
{
   qDebug() << QT_VERSION_STR;
   Object o;
   o.connect(&o, SIGNAL(source(Data)), SLOT(sink(Data)));
   emit o.source(Data(2));
   return 0;
}


#include "main.moc"
