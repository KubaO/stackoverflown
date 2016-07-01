// https://github.com/KubaO/stackoverflown/tree/master/questions/surface-20737882
#include <QtWidgets>

class WidgetMonitor : public QObject
{
   Q_OBJECT
   QSet<QWidget *> m_awake;
   QBasicTimer m_timer;
   int m_counter = 0;
   void queue(QWidget * w) {
      m_awake << w->window();
      if (! m_timer.isActive()) m_timer.start(0, this);
   }
   bool eventFilter(QObject * obj, QEvent * ev) {
      switch (ev->type()) {
      case QEvent::Paint: {
         if (! obj->isWidgetType()) break;
         queue(static_cast<QWidget*>(obj));
         break;
      }
      case QEvent::ChildAdded: {
         if (obj->isWidgetType()) obj->installEventFilter(this);
         break;
      }
      default: break;
      }
      return false;
   }
   void timerEvent(QTimerEvent * ev) {
      if (ev->timerId() != m_timer.timerId()) return;
      qDebug() << "painting: " << m_counter++ << m_awake;
      for (auto w : m_awake) {
         auto img = dynamic_cast<QImage*>(w->backingStore()->paintDevice());
         if (img) emit newContents(img, w);
      }
      m_awake.clear();
      m_timer.stop();
   }
   Q_SLOT void windowDestroyed(QObject * obj) {
      if (! obj->isWidgetType()) return;
      m_awake.remove(static_cast<QWidget*>(obj));
   }
public:
   explicit WidgetMonitor(QObject * parent = nullptr) : QObject{parent} {}
   explicit WidgetMonitor(QWidget * w, QObject * parent = nullptr) : QObject{parent} {
      monitor(w);
   }
   Q_SLOT void monitor(QWidget * w) {
      w = w->window();
      connect(w, &QObject::destroyed, this, &WidgetMonitor::windowDestroyed);
      w->installEventFilter(this);
      for (auto obj : w->children())
         if (obj->isWidgetType()) obj->installEventFilter(this);
      queue(w);
   }
   Q_SLOT void unMonitor(QWidget * w) {
      w = w->window();
      disconnect(w, &QObject::destroyed, this, &WidgetMonitor::windowDestroyed);
      m_awake.remove(w);
      w->removeEventFilter(this);
      for (auto obj : w->children()) {
         if (obj->isWidgetType()) obj->removeEventFilter(this);
      }
   }
   Q_SIGNAL void newContents(const QImage *, QWidget * w);
};

class TestWidget : public QWidget {
   QVBoxLayout m_layout{this};
   QLabel m_time;
   QBasicTimer m_timer;
   void timerEvent(QTimerEvent * ev) override {
      if (ev->timerId() != m_timer.timerId()) return;
      m_time.setText(QTime::currentTime().toString());
   }
public:
   explicit TestWidget(QWidget * parent = 0) : QWidget{parent} {
      m_layout.addWidget(&m_time);
      m_layout.addWidget(new QLabel{"Static Label"});
      m_layout.addWidget(new QPushButton{"A Button"});
      m_timer.start(1000, this);
   }
};

int main(int argc, char **argv)
{
   QApplication app{argc, argv};
   TestWidget src;
   QWidget dst;
   QVBoxLayout dl{&dst};
   QLabel description{"Destination"};
   QLabel contents;
   contents.setFrameShape(QFrame::Box);
   dl.addWidget(&description);
   dl.addWidget(&contents);
   src.show();
   dst.show();

   WidgetMonitor mon(&src);
   QObject::connect(&mon, &WidgetMonitor::newContents, [&](const QImage * img){
      contents.resize(img->size());
      contents.setPixmap(QPixmap::fromImage(*img));
   });
   return app.exec();
}

#include "main.moc"
