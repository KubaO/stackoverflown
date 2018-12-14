// https://github.com/KubaO/stackoverflown/tree/master/questions/surface-20737882
#include <QtWidgets>
#include <array>

const char kFiltered[] = "WidgetMonitor_filtered";

class WidgetMonitor : public QObject {
   Q_OBJECT
   QVector<QPointer<QWidget>> m_awake;
   QBasicTimer m_timer;
   int m_counter = 0;
   void queue(QWidget *window) {
      Q_ASSERT(window && window->isWindow());
      if (!m_awake.contains(window)) m_awake << window;
      if (!m_timer.isActive()) m_timer.start(0, this);
   }
   void filter(QObject *obj) {
      if (obj->isWidgetType() && !obj->property(kFiltered).toBool()) {
         obj->installEventFilter(this);
         obj->setProperty(kFiltered, true);
      }
   }
   void unfilter(QObject *obj) {
      if (obj->isWidgetType() && obj->property(kFiltered).toBool()) {
         obj->removeEventFilter(this);
         obj->setProperty(kFiltered, false);
      }
   }
   bool eventFilter(QObject *obj, QEvent *ev) override {
      switch (ev->type()) {
         case QEvent::Paint: {
            if (!obj->isWidgetType()) break;
            if (auto *window = static_cast<QWidget *>(obj)->window()) queue(window);
            break;
         }
         case QEvent::ChildAdded: {
            auto *cev = static_cast<QChildEvent *>(ev);
            if (auto *child = qobject_cast<QWidget *>(cev->child())) monitor(child);
            break;
         }
         default:
            break;
      }
      return false;
   }
   void timerEvent(QTimerEvent *ev) override {
      if (ev->timerId() != m_timer.timerId()) return;
      qDebug() << "painting: " << m_counter++ << m_awake;
      for (auto w : m_awake)
         if (auto *img = dynamic_cast<QImage *>(w->backingStore()->paintDevice()))
            emit newContents(*img, w);
      m_awake.clear();
      m_timer.stop();
   }

  public:
   explicit WidgetMonitor(QObject *parent = nullptr) : QObject{parent} {}
   explicit WidgetMonitor(QWidget *w, QObject *parent = nullptr) : QObject{parent} {
      monitor(w);
   }
   Q_SLOT void monitor(QWidget *w) {
      w = w->window();
      if (!w) return;
      filter(w);
      for (auto *obj : w->findChildren<QWidget *>()) filter(obj);
      queue(w);
   }
   Q_SLOT void unMonitor(QWidget *w) {
      w = w->window();
      if (!w) return;
      unfilter(w);
      for (auto *obj : w->findChildren<QWidget *>()) unfilter(obj);
      m_awake.removeAll(w);
   }
   Q_SIGNAL void newContents(const QImage &, QWidget *w);
};

class TestWidget : public QWidget {
   QVBoxLayout m_layout{this};
   QLabel m_time;
   QBasicTimer m_timer;
   void timerEvent(QTimerEvent *ev) override {
      if (ev->timerId() != m_timer.timerId()) return;
      m_time.setText(QTime::currentTime().toString());
   }

  public:
   explicit TestWidget(QWidget *parent = nullptr) : QWidget{parent} {
      m_layout.addWidget(&m_time);
      m_layout.addWidget(new QLabel{"Static Label"});
      m_layout.addWidget(new QPushButton{"A Button"});
      m_timer.start(1000, this);
   }
};

int main(int argc, char **argv) {
   QApplication app{argc, argv};
   TestWidget src;
   QLabel dst;
   dst.setFrameShape(QFrame::Box);
   for (auto *w : std::array<QWidget *, 2>{&dst, &src}) {
      w->show();
      w->raise();
   }
   QMetaObject::invokeMethod(&dst, [&] { dst.move(src.frameGeometry().topRight()); },
                             Qt::QueuedConnection);

   WidgetMonitor mon(&src);
   src.setWindowTitle("Source");
   dst.setWindowTitle("Destination");
   QObject::connect(&mon, &WidgetMonitor::newContents, [&](const QImage &img) {
      dst.resize(img.size());
      dst.setPixmap(QPixmap::fromImage(img));
   });
   return app.exec();
}

#include "main.moc"
