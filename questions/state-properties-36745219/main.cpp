// https://github.com/KubaO/stackoverflown/tree/master/questions/state-properties-36745219
#include <QtWidgets>

class NamedState : public QState {
   Q_OBJECT
public:
   NamedState(const char * name, QStateMachine * parent) : QState(parent) {
      setObjectName(QString::fromUtf8(name));
   }
};

struct Transition : public QObject {
   Q_OBJECT
public:
   Transition(QState * source, QState * destination) : QObject(source->machine()) {
      source->addTransition(this, &Transition::trigger, destination);
   }
   Q_SIGNAL void trigger();
};

class Delay : public Transition {
   Q_OBJECT
   int m_delay;
   QBasicTimer m_timer;
   void timerEvent(QTimerEvent * ev) {
      if (m_timer.timerId() != ev->timerId()) return;
      m_timer.stop();
      trigger();
   }
public:
   Delay(QState * s, QState * d, int ms) : Transition(s, d), m_delay(ms) {
      connect(s, &QState::entered, this, [this]{ m_timer.start(m_delay, this);});
   }
};

class MeasController : public QObject {
   Q_OBJECT
   QStateMachine m_machine{this};
   NamedState
      m_idle    {"Idle", &m_machine},
      m_start_z {"Start Z", &m_machine},
      m_active_z{"Active Z", &m_machine},
      m_downMove{"DownMove", &m_machine};
   Transition
      m_toStartZ{&m_idle, &m_start_z},
      m_toDownMove{&m_active_z, &m_downMove};
   Delay
      m_d1{&m_start_z, &m_active_z, 1000},
      m_d2{&m_downMove, &m_active_z, 2000};
   QList<QState*> states() const { return m_machine.findChildren<QState*>(); }
public:
   MeasController(QObject * parent = 0) : QObject(parent) {
      for (auto state : states())
         connect(state, &QState::entered, [state]{ qDebug() << state->objectName(); });
      m_machine.setInitialState(&m_idle);
      m_machine.start();
   }
   Q_SLOT void startZ() { m_toStartZ.trigger(); }
   Q_SLOT void moveDown() { m_toDownMove.trigger(); }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   MeasController ctl;
   QWidget w;
   QGridLayout layout{&w};
   QPushButton start{"Start"};
   QPushButton moveDown{"Move Down"};
   QPlainTextEdit log;
   log.setReadOnly(true);
   layout.addWidget(&start, 0, 0);
   layout.addWidget(&moveDown, 0, 1);
   layout.addWidget(&log, 1, 0, 1, 2);

   QObject::connect(&start, &QPushButton::clicked, &ctl, &MeasController::startZ);
   QObject::connect(&moveDown, &QPushButton::clicked, &ctl, &MeasController::moveDown);

   static QtMessageHandler handler = qInstallMessageHandler(
      +[](QtMsgType t, const QMessageLogContext& c, const QString & msg){
      static QPointer<QPlainTextEdit> log{[]{
         for (auto w : qApp->topLevelWidgets())
            for (auto log : w->findChildren<QPlainTextEdit*>()) return log;
         Q_ASSERT(false);
      }()};
      if (log) log->appendPlainText(msg);
      handler(t, c, msg);
   });

   w.show();
   return app.exec();
}

#include "main.moc"
