#include <QApplication>
#include <QList>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QBasicTimer>
#include <QLabel>
#include <stdint.h>

#if 0
#include "leap.h"
#else

namespace Leap {
// Dummy Leap Implementation

class Controller;
class Listener {
   Q_DISABLE_COPY(Listener)
public:
   Listener() {}
   virtual void onFrame(const Controller &) {}
};

class Frame {
   friend class Controller;
   int64_t m_id;
   Frame(int64_t id) : m_id(id) {}
public:
   int64_t id() const { return m_id; }
};

class Controller {
   QMutex mutable m_mutex;
   QList<Frame> m_frames;
   QList<Listener*> m_listeners;
   int64_t m_frameId;
   struct Impl : public QObject {
      Controller * const q;
      QBasicTimer m_timer;
      Impl(Controller * ctl) : q(ctl) { m_timer.start(1000/60, this); }
      void timerEvent(QTimerEvent * ev) {
         if (ev->timerId() != m_timer.timerId()) return;
         QMutexLocker lock(&q->m_mutex);
         QList<Listener*> listeners = q->m_listeners;
         if (q->m_frames.size() >= 60) q->m_frames.removeFirst();
         q->m_frames << Frame(q->m_frameId++);
         lock.unlock();
         foreach (Listener * l, listeners) { l->onFrame(*q); }
      }
   } m_impl;
   struct Thread : public QThread {
      ~Thread() { quit(); wait(); }
   } m_thread;
public:
   Controller() : m_frameId(0), m_impl(this) {
      m_thread.start();
      m_impl.moveToThread(&m_thread);
   }
   bool addListener(Listener & l) {
      QMutexLocker lock(&m_mutex);
      if (m_listeners.contains(&l)) return false;
      m_listeners << &l;
      return true;
   }
   bool removeListener(Listener & l) {
      QMutexLocker lock(&m_mutex);
      return m_listeners.removeAll(&l);
   }
   Frame frame(int history = 0) const {
      QMutexLocker lock(&m_mutex);
      if (history < m_frames.size()) {
         return m_frames.at(m_frames.size() - 1 - history);
      }
      return Frame(-1); // "invalid" frame
   }
};

}
#endif

// An example of how to use Leap

class MyListener : public QObject, public Leap::Listener {
public:
   virtual void onFrame(const Leap::Controller & ctl) {
      Leap::Frame f = ctl.frame();
      // This is a hack so that we avoid having to declare a signal and
      // use moc generated code.
      setObjectName(QString::number(f.id()));
      // emits objectNameChanged(QString)
   }
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   MyListener listener;
   Leap::Controller controller;
   controller.addListener(listener);

   QLabel frameLabel;
   frameLabel.setMinimumSize(200, 50);
   frameLabel.show();
   frameLabel.connect(&listener, SIGNAL(objectNameChanged(QString)),
                      SLOT(setText(QString)));

   int rc = a.exec();
   controller.removeListener(listener);
   return rc;
}
