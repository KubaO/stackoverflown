// https://github.com/KubaO/stackoverflown/tree/master/questions/robot-state-timer-36769933
#include <QtWidgets>

struct Transition : public QObject {
   Q_OBJECT
public:
   Transition(QState * source, QState * destination) : QObject(source->machine()) {
      source->addTransition(this, &Transition::trigger, destination);
   }
   Q_SIGNAL void trigger();
   void operator()() { trigger(); }
};

class Delay : public Transition {
   Q_OBJECT
   int m_duration;
   QBasicTimer m_timer;
   void timerEvent(QTimerEvent * ev) {
      if (m_timer.timerId() != ev->timerId()) return;
      m_timer.stop();
      trigger();
   }
public:
   Delay(QState * src, QState * dst, int ms) : Transition(src, dst), m_duration(ms) {
      connect(src, &QState::entered, this, [this]{ m_timer.start(m_duration, this);});
   }
   Q_SLOT void setDuration(int duration) { m_duration = duration; }
};

class Controller : public QObject {
   Q_OBJECT
   QStateMachine m_machine{this};
   QState
      m_idle    {&m_machine},
      m_active  {&m_machine};
   Transition
      m_go{&m_idle, &m_active};
   Delay
      m_activeTime{&m_active, &m_idle, 0};
public:
   Controller(QObject * parent = 0) : QObject(parent) {
      connect(&m_idle, &QState::entered, this, &Controller::isIdle);
      connect(&m_active, &QState::entered, this, &Controller::isActive);
      m_machine.setInitialState(&m_idle);
      m_machine.start();
   }
   Q_SLOT void moveFor(int ms) {
      m_activeTime.setDuration(ms);
      m_go();
   }
   Q_SIGNAL void isIdle();
   Q_SIGNAL void isActive();
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   Controller ctl;
   QWidget w;
   QFormLayout layout{&w};
   QPushButton start{"Go"};
   QLineEdit duration{"5000"};
   QPlainTextEdit log;
   log.setReadOnly(true);
   layout.addRow("Duration", &duration);
   layout.addRow(&start);
   layout.addRow(&log);

   QObject::connect(&ctl, &Controller::isIdle, &log, [&]{ log.appendPlainText("Idle"); });
   QObject::connect(&ctl, &Controller::isActive, &log, [&]{ log.appendPlainText("Active"); });
   QObject::connect(&start, &QPushButton::clicked, &ctl, [&]{
      ctl.moveFor(duration.text().toInt());
   });
   w.show();
   return app.exec();
}

#include "main.moc"
