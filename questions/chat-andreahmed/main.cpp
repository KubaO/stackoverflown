// https://github.com/KubaO/stackoverflown/tree/master/questions/chat-andreahmed
#include <QtCore>

//! A value in a certain unit of measurement. This is a value class. The signal/slot
//! connections are not modified when copy-constructing or assigning.
//! See http://stackoverflow.com/a/25939236/1329652 for discussion of a copyable
//! QObject.
class Value : public QObject {
   Q_OBJECT
   qreal m_value {0.0f};
   int m_unit {0};
public:
   Q_SIGNAL void valueChanged(qreal);
   Value() {}
   Value(int unit, QObject *parent) : QObject(parent), m_unit(unit) {}
   Value(const Value &other) : m_value(other.m_value), m_unit(other.m_unit) {
      setObjectName(other.objectName());
   }
   // !! http://stackoverflow.com/questions/3279543/what-is-the-copy-and-swap-idiom
   friend void swap(Value & first, Value & second) {
      using std::swap;

      auto tempName = first.objectName();
      first.setObjectName(second.objectName());
      second.setObjectName(tempName);

      auto tempValue = first.m_value;
      first.setValue(second.m_value);
      second.setValue(tempValue);

      swap(first.m_unit, second.m_unit);
   }
   Value(Value && other) { swap(*this, other); }
   Value & operator=(Value other) {
      swap(*this, other);
      return *this;
   }
   Value & operator=(const qreal value) {
      setValue(value);
      return *this;
   }
   operator qreal() const { return m_value; }
   qreal value() const { return m_value; }
   void setValue(qreal value) {
      if (m_value != value) {
         m_value = value;
         emit valueChanged(m_value);
      }
   }
   void loadFrom(QSettings &set, bool ownGroup)
   {
      Q_UNUSED(ownGroup)
      if(set.contains(objectName())) {
         m_value = set.value(objectName(),0.0).toDouble();
         emit valueChanged(m_value);
      }
   }
   void saveTo(QSettings &set, bool ownGroup, bool force)
   {
      Q_UNUSED(ownGroup)
      Q_UNUSED(force)
      set.setValue(objectName(),m_value);
   }
};
Q_DECLARE_METATYPE(Value)

QDataStream &operator<<(QDataStream &out, const Value &unit)
{
   out << unit.value();
   return out;
}

QDataStream &operator>>(QDataStream &in, Value &unit)
{
   double value;
   in >> value;
   unit.setValue(value);
   return in;
}

class Profile : public QObject
{
   Q_OBJECT
   Q_PROPERTY(Value Ta READ Ta NOTIFY TaChanged)
   Q_PROPERTY(Value Te READ Te NOTIFY TeChanged)
   Q_PROPERTY(Value Texp READ Texp  NOTIFY TexpChanged)
   Q_PROPERTY(Value offset READ offset NOTIFY offsetChanged)
   Q_PROPERTY(float lim1 READ lim1 WRITE setLim1 NOTIFY lim1Changed)
   Q_PROPERTY(float lim2 READ lim2 WRITE setLim2 NOTIFY lim2Changed)
   Q_PROPERTY(QString profilename READ profilename WRITE setProfilename NOTIFY profilenameChanged)

   Value  m_Ta;
   Value  m_Te;
   Value  m_offset;
   Value  m_Texp;
   float m_lim1;
   float m_lim2;
   QString m_profilename;

   template <typename V, typename F> void set(V & member, const V & value, F method) {
      if (member == value) return;
      member = value;
      (this->*method)(value);
   }

public:
   Profile() {}
   Profile(const QString & name, QObject * parent = 0) : QObject(parent), m_profilename(name) {}
   const Value & Ta() const { return m_Ta; }
   const Value & Te() const { return m_Te; }
   const Value & offset() const { return m_offset; }
   const Value & Texp() const { return m_Texp; }
   float lim1() const { return m_lim1; }
   float lim2() const { return m_lim2; }
   QString profilename() const { return m_profilename; }
   void save(QSettings& settings) const;
   void load(QSettings& settings);

public slots:
   void setLim1(float lim1) { set(m_lim1, lim1, &Profile::lim2Changed); }
   void setLim2(float lim2) { set(m_lim2, lim2, &Profile::lim2Changed); }
   void setProfilename(QString profilename) { set(m_profilename, profilename, &Profile::profilenameChanged); }
   void calculateTexpected() {}

signals:
   void TaChanged(float Ta);
   void TeChanged(float Te);
   void TexpChanged(float Texp);
   void offsetChanged(float offset);
   void lim1Changed(float lim1);
   void lim2Changed(float lim2);
   void profilenameChanged(QString profilename);
};

void Profile::save(QSettings& settings) const {
   for(int i=0; i<metaObject()->propertyCount(); ++i) {
      const auto& p = metaObject()->property(i);
      if (p.isStored(this)) {
         settings.setValue(p.name(), property(p.name()));
      }
   }
}

void Profile::load(QSettings& settings) {
   for(int i=0; i<metaObject()->propertyCount(); ++i) {
      const auto& p = metaObject()->property(i);
      if(p.isStored(this)) {
         setProperty(p.name(), settings.value(p.name()));
      }
   }
}

int main(int argc, char ** argv) {
   QCoreApplication app(argc, argv);
   app.setApplicationName("ProfileTest");
   app.setOrganizationDomain("stackoverflow.com");
   qRegisterMetaType<Value>();
   qRegisterMetaTypeStreamOperators<Value>();
   QSettings s;
   Profile p1, p2;
   p1.setLim1(10.0f);
   p1.setLim2(20.0f);
   p1.save(s);
   p2.load(s);
   Q_ASSERT(p2.lim1() == 10.0f && p2.lim2() == 20.0f);
}

#include "main.moc"
