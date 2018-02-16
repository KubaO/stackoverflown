// https://github.com/KubaO/stackoverflown/tree/master/questions/chattytimer-25695203
// chattytimer.h
#pragma once
#include <QAbstractEventDispatcher>
#include <QBasicTimer>
#include <QTimerEvent>
class ChattyTimer : public QObject {
   Q_OBJECT
   Q_PROPERTY(bool active READ isActive)
   Q_PROPERTY(int remainingTime READ remainingTime)
   Q_PROPERTY(int interval READ interval WRITE setInterval)
   Q_PROPERTY(bool singleShot READ singleShot WRITE setSingleShot)
   Q_PROPERTY(Qt::TimerType timerType READ timerType WRITE setTimerType)
   Q_PROPERTY(bool immediateStopTimeout READ immediateStopTimeout WRITE setImmediateStopTimeout)
   Q_PROPERTY(bool activeUntilLastTimeout READ activeUntilLastTimeout WRITE setActiveUntilLastTimeout)
   Qt::TimerType m_type = Qt::CoarseTimer;
   bool m_singleShot = false;
   bool m_stopTimeout = false;
   bool m_immediateStopTimeout = false;
   bool m_activeUntilLastTimeout = false;
   QBasicTimer m_timer;
   int m_interval = 0;
   void timerEvent(QTimerEvent * ev) override {
      if (ev->timerId() != m_timer.timerId()) return;
      if (m_singleShot || m_stopTimeout) m_timer.stop();
      m_stopTimeout = false;
      emit timeout({});
   }
public:
   ChattyTimer(QObject * parent = {}) : QObject(parent) {}
   Q_SLOT void start(int msec) {
      m_interval = msec;
      start();
   }
   Q_SLOT void start() {
      m_stopTimeout = false;
      m_timer.stop(); // don't emit the signal here
      m_timer.start(m_interval, m_type, this);
   }
   Q_SLOT void stop() {
      if (!isActive()) return;
      m_timer.stop();
      m_stopTimeout = !m_immediateStopTimeout;
      if (m_immediateStopTimeout)
         emit timeout({});
      else // defer to the event loop
         m_timer.start(0, this);
   }
   Q_SIGNAL void timeout(QPrivateSignal);
   int timerId() const {
      return isActive() ? m_timer.timerId() : -1;
   }
   bool isActive() const {
      return m_timer.isActive() && (m_activeUntilLastTimeout || !m_stopTimeout);
   }
   int remainingTime() const {
      return
            isActive()
            ? QAbstractEventDispatcher::instance()->remainingTime(m_timer.timerId())
            : -1;
   }
   int interval() const { return m_interval; }
   void setInterval(int msec) {
      m_interval = msec;
      if (!isActive()) return;
      m_timer.stop(); // don't emit the signal here
      start();
   }
   bool singleShot() const { return m_singleShot; }
   void setSingleShot(bool s) { m_singleShot = s; }
   Qt::TimerType timerType() const { return m_type; }
   void setTimerType(Qt::TimerType t) { m_type = t; }
   bool immediateStopTimeout() const { return m_immediateStopTimeout; }
   void setImmediateStopTimeout(bool s) { m_immediateStopTimeout = s; }
   bool activeUntilLastTimeout() const { return m_activeUntilLastTimeout; }
   void setActiveUntilLastTimeout(bool s) { m_activeUntilLastTimeout = s; }
};
