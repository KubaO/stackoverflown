// https://github.com/KubaO/stackoverflown/tree/master/questions/qml-rate-limter-42284163
#include <QtCore>

class PropertyRateLimiter : public QObject {
   Q_OBJECT
   qint64 msecsPeriod{500};
   const QByteArray property;
   bool dirty{};
   QVariant value;
   QElapsedTimer time;
   QBasicTimer timer;
   QMetaMethod slot = metaObject()->method(metaObject()->indexOfSlot("onChange()"));
   void signal() {
      if (time.isValid()) time.restart(); else time.start();
      if (dirty)
         emit valueChanged(value, parent(), property);
      else
         timer.stop();
      dirty = false;
   }
   Q_SLOT void onChange() {
      dirty = true;
      value = parent()->property(property);
      auto elapsed = time.isValid() ? time.elapsed() : 0;
      if (!time.isValid() || elapsed >= msecsPeriod)
         signal();
      else
         if (!timer.isActive())
            timer.start(msecsPeriod - elapsed, this);
   }
   void timerEvent(QTimerEvent *event) override {
      if (timer.timerId() == event->timerId())
         signal();
   }
public:
   PropertyRateLimiter(const char * propertyName, QObject * parent) :
      QObject{parent}, property{propertyName}
   {
      auto mo = parent->metaObject();
      auto property = mo->property(mo->indexOfProperty(this->property));
      if (!property.hasNotifySignal())
         return;
      connect(parent, property.notifySignal(), this, slot);
   }
   void setPeriod(int period) { msecsPeriod = period; }
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
    PropertyRateLimiter limiter("position", slider);
    QObject::connect(&limiter, &PropertyRateLimiter::valueChanged, [&](const QVariant & val){
       label->setProperty("text", val);
    });
    return app.exec();
}
