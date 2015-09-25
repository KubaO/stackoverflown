// https://github.com/KubaO/stackoverflown/tree/master/questions/event-signaler-32648234
#include <QtWidgets>

struct EventWrapper {
   QPointer<QObject> target;
   QEvent::Type type { QEvent::None };
   QEvent * event { nullptr };
public:
   EventWrapper() {}
   EventWrapper(QObject * target, QEvent * event) :
      target(target), type(event->type()), event(event) {}
   EventWrapper(const EventWrapper & o) : target(o.target), type(o.type) {}
   EventWrapper(EventWrapper && o) :
      target(o.target), type(o.type), event(o.event) { o.event = nullptr; }
   EventWrapper & operator=(const EventWrapper & o) {
      target = o.target;
      type = o.type;
      event = nullptr;
      return *this;
   }
};
Q_DECLARE_METATYPE(EventWrapper)

class EventSignaler : public QObject {
   Q_OBJECT
   QMap<QObject*, QSet<int>> m_watch;
   bool eventFilter(QObject * obj, QEvent * ev) Q_DECL_OVERRIDE {
      auto it = m_watch.find(obj);
      if (it != m_watch.end() && it.value().contains(ev->type()))
         emit eventSignal(EventWrapper(obj, ev));
      return false;
   }
public:
   EventSignaler(QObject * parent = 0) : QObject(parent) {}
   void watch(QObject * object, QEvent::Type type) {
      auto it = m_watch.find(object);
      if (it == m_watch.end()) {
         it = m_watch.insert(object, QSet<int>() << type);
         object->installEventFilter(this);
         connect(object, &QObject::destroyed, this, [this, object]{
            m_watch.remove(object);
         });
      } else
         it.value().insert(type);
   }
   void unWatch(QObject * object, QEvent::Type type) {
      auto it = m_watch.find(object);
      if (it == m_watch.end()) return;
      it.value().remove(type);
      if (it.value().isEmpty()) m_watch.erase(it);
   }
   Q_SIGNAL void eventSignal(const EventWrapper &);
};

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   EventSignaler signaler;
   QWidget w;
   QVBoxLayout layout(&w);
   QLabel label("text");
   QLineEdit edit;
   layout.addWidget(&label);
   layout.addWidget(&edit);
   signaler.watch(&label, QEvent::MouseButtonPress);
   QObject::connect(&signaler, &EventSignaler::eventSignal,
                    [&label, &edit]{ label.setText(edit.text()); });
   w.show();
   return app.exec();
}

#include "main.moc"
