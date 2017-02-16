// https://github.com/KubaO/stackoverflown/tree/master/questions/qml-rate-limter-42284163
#include <QtCore>
#include <forward_list>

class PropertyRateLimiter : public QObject {
   Q_OBJECT
   qint64 m_msecsPeriod{500};
   struct Data {
      bool dirty{};
      QObject * object;
      QVariant value;
      QElapsedTimer time;
      QBasicTimer timer;
      void signal(PropertyRateLimiter * limiter) {
         if (time.isValid()) time.restart(); else time.start();
         if (dirty)
            emit limiter->valueChanged(value, object, limiter->m_property);
         else
            timer.stop();
         dirty = false;
      }
      Data() {}
      Q_DISABLE_COPY(Data)
   };
   std::forward_list<Data> m_data;
   const QByteArray m_property;
   QMetaMethod slot = metaObject()->method(metaObject()->indexOfSlot("onChange()"));
   Data & dataFor(QObject * object) {
      for (auto & data : m_data)
         if (data.object == object)
            return data;
      m_data.emplace_front();
      m_data.front().object = object;
      return m_data.front();
   }
   Q_SLOT void onChange() {
      auto & data = dataFor(sender());
      data.dirty = true;
      data.value = data.object->property(m_property);
      auto elapsed = data.time.isValid() ? data.time.elapsed() : 0;
      if (!data.time.isValid() || elapsed >= m_msecsPeriod)
         data.signal(this);
      else
         if (!data.timer.isActive())
            data.timer.start(m_msecsPeriod - elapsed, this);
   }
   void timerEvent(QTimerEvent *event) override {
      for (auto & data : m_data) {
         if (data.timer.timerId() != event->timerId())
            continue;
         data.signal(this);
      }
   }
public:
   PropertyRateLimiter(const char * propertyName, QObject * parent = nullptr) :
      QObject{parent}, m_property{propertyName} {}
   void watch(QObject * object) {
      auto mo = object->metaObject();
      auto property = mo->property(mo->indexOfProperty(m_property));
      if (!property.hasNotifySignal())
         return;
      connect(object, &QObject::destroyed, this, [this, object]{
         m_data.remove_if([object](const Data & p){ return p.object == object; });
      });
      connect(object, property.notifySignal(), this, slot);
   }
   Q_SIGNAL void valueChanged(const QVariant &, QObject *, const QByteArray & name);
};
#include "main.moc"

#include <QtQuick>

const char qmlData[] =
R"__end(
import QtQuick 2.6
import QtQuick.Controls 2.0

ApplicationWindow {
   minimumWidth: 300
   minimumHeight: 250
   visible: true

   Column {
      anchors.fill: parent
      Slider { objectName: "slider" }
      Label { objectName: "label" }
   }
}
)__end";

int main(int argc, char ** argv) {
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app{argc, argv};
    QQmlApplicationEngine engine;
    engine.loadData(QByteArray::fromRawData(qmlData, sizeof(qmlData)-1));
    auto window = engine.rootObjects().first();
    auto slider = window->findChild<QObject*>("slider");
    auto label = window->findChild<QObject*>("label");
    PropertyRateLimiter limiter("position");
    limiter.watch(slider);
    QObject::connect(&limiter, &PropertyRateLimiter::valueChanged, [&](const QVariant & val){
       label->setProperty("text", val);
    });
    return app.exec();
}
