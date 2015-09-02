#include <QGuiApplication>
#include <QtQml>

class HardwareComponent : public QObject {
   Q_OBJECT
   Q_PROPERTY(QString category MEMBER m_category READ category CONSTANT)
   Q_PROPERTY(HardwareComponent * cpu READ cpu WRITE setCpu NOTIFY cpuChanged)
   Q_PROPERTY(int price READ price NOTIFY priceChanged)
   Q_PROPERTY(int unitPrice MEMBER m_unitPrice READ unitPrice WRITE setUnitPrice NOTIFY unitPriceChanged)
   QString m_category;
   int m_unitPrice;
   bool event(QEvent * ev) Q_DECL_OVERRIDE {
      if (ev->type() != QEvent::ChildAdded && ev->type() != QEvent::ChildRemoved)
         return QObject::event(ev);
      auto childEvent = static_cast<QChildEvent*>(ev);
      auto child = qobject_cast<HardwareComponent*>(childEvent->child());
      if (! child) return QObject::event(ev);
      if (childEvent->added())
         connect(child, &HardwareComponent::priceChanged,
                 this, &HardwareComponent::priceChanged, Qt::UniqueConnection);
      else
         disconnect(child, &HardwareComponent::priceChanged,
                    this, &HardwareComponent::priceChanged);
      emit priceChanged(price());
      if (child->category() == "CPU") emit cpuChanged(cpu());
      return QObject::event(ev);
   }
public:
   HardwareComponent(int price, QString category = QString(), QObject * parent = 0) :
      QObject(parent), m_category(category), m_unitPrice(price) {}
   HardwareComponent * cpu() const {
      for (auto child : findChildren<HardwareComponent*>())
         if (child->category() == "CPU") return child;
      return 0;
   }
   Q_INVOKABLE void setCpu(HardwareComponent * newCpu) {
      Q_ASSERT(!newCpu || newCpu->category() == "CPU");
      auto oldCpu = cpu();
      if (oldCpu == newCpu) return;
      if (oldCpu) oldCpu->setParent(0);
      if (newCpu) newCpu->setParent(this);
      emit cpuChanged(newCpu);
   }
   Q_SIGNAL void cpuChanged(HardwareComponent *);
   virtual int price() const {
      int total = unitPrice();
      for (auto child : findChildren<HardwareComponent*>(QString(), Qt::FindDirectChildrenOnly))
         total += child->price();
      return total;
   }
   Q_SIGNAL void priceChanged(int);
   int unitPrice() const { return m_unitPrice; }
   void setUnitPrice(int unitPrice) {
      if (m_unitPrice == unitPrice) return;
      m_unitPrice = unitPrice;
      emit unitPriceChanged(m_unitPrice);
      emit priceChanged(this->price());
   }
   Q_SIGNAL void unitPriceChanged(int);
   QString category() const { return m_category; }
};

struct Computer : public HardwareComponent {
   Computer() : HardwareComponent(400) {}
};

class FluctuatingPriceComponent : public HardwareComponent {
   QTimer m_timer;
   int m_basePrice;
public:
   FluctuatingPriceComponent(int basePrice, const QString & category = QString(), QObject * parent = 0) :
      HardwareComponent(basePrice, category, parent),
      m_basePrice(basePrice) {
      m_timer.start(250);
      connect(&m_timer, &QTimer::timeout, [this]{
         setUnitPrice(m_basePrice + qrand()*20.0/RAND_MAX - 10);
      });
   }
};

int main(int argc, char *argv[])
{
   QGuiApplication app(argc, argv);
   QQmlApplicationEngine engine;
   Computer computer;
   HardwareComponent memoryBay(40, "Memory Bay", &computer);
   HardwareComponent memoryStick(60, "Memory Stick", &memoryBay);
   FluctuatingPriceComponent cpu1(100, "CPU", &computer);
   HardwareComponent cpu2(200, "CPU");

   qmlRegisterUncreatableType<HardwareComponent>("bar.foo", 1, 0, "HardwareComponent", "");
   engine.rootContext()->setContextProperty("computer", &computer);
   engine.rootContext()->setContextProperty("cpu1", &cpu1);
   engine.rootContext()->setContextProperty("cpu2", &cpu2);
   engine.load(QUrl("qrc:/main.qml"));
   return app.exec();
}

#include "main.moc"
