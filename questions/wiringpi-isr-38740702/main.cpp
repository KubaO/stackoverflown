// https://github.com/KubaO/stackoverflown/tree/master/questions/wiringpi-isr-38740702
#include <QtWidgets>
#include <wiringpi.h>

class Controller : public QObject {
   Q_OBJECT
   QStateMachine m_mach{this};
   QState s_stopped{&m_mach};
   QState s_moving {&m_mach};
   QState s_forward{&s_moving};
   QState s_reverse{&s_moving};
   static QPointer<Controller> m_instance;
   enum { sensorPin = 24, motorPinA = 10, motorPinB = 11 };
   // These methods use digitalWrite() to control the motor
   static void motorForward() {
      digitalWrite(motorPinA, HIGH);
      digitalWrite(motorPinB, LOW);
   }
   static void motorReverse() { /*...*/ }
   static void motorStop() { /*...*/ }
   //
   Q_SIGNAL void toStopped();
   Q_SIGNAL void toForward();
   Q_SIGNAL void toReverse();
   void setupIO() {
      wiringPiSetupSys();
      pinMode(sensorPin, INPUT);
      wiringPiISR(sensorPin, INT_EDGE_RISING, &Controller::sensorHit);
   }
   /// This method is safe to be called from any thread.
   static void sensorHit() {
      motorReverse(); // do it right away in the high-priority thread
      emit m_instance->toReverse();
   }
public:
   Controller(QObject * parent = nullptr) : QObject{parent} {
      Q_ASSERT(!m_instance);
      // State Machine Definition
      m_mach.setInitialState(&s_stopped);
      s_stopped.addTransition(this, &Controller::toForward, &s_forward);
      s_moving.addTransition (this, &Controller::toStopped, &s_stopped);
      s_forward.addTransition(this, &Controller::toReverse, &s_reverse);
      s_reverse.addTransition(this, &Controller::toForward, &s_forward);
      connect(&s_stopped, &QState::entered, this, [this]{
         motorStop();
         emit isStopped();
      });
      connect(&s_forward, &QState::entered, this, [this]{
         motorForward();
         emit isForward();
      });
      connect(&s_reverse, &QState::entered, this, [this]{
         motorReverse();
         emit isReverse();
      });
      m_mach.start();
      //
      m_instance = this;
      setupIO();
   }
   Q_SLOT void forward() { emit toForward(); }
   Q_SLOT void stop() {
      motorStop(); // do it right away to ensure we stop ASAP
      emit toStopped();
   }
   Q_SIGNAL void isStopped();
   Q_SIGNAL void isForward();
   Q_SIGNAL void isReverse();
};
QPointer<Controller> Controller::m_instance;

int main(int argc, char ** argv) {
   using Q = QObject;
   QApplication app{argc, argv};
   Controller ctl;
   QWidget ui;
   QVBoxLayout layout{&ui};
   QLabel state;
   QPushButton move{"Move Forward"};
   QPushButton stop{"Stop"};
   layout.addWidget(&state);
   layout.addWidget(&move);
   layout.addWidget(&stop);
   Q::connect(&ctl, &Controller::isStopped, &state, [&]{ state.setText("Stopped"); });
   Q::connect(&ctl, &Controller::isForward, &state, [&]{ state.setText("Forward"); });
   Q::connect(&ctl, &Controller::isReverse,  &state, [&]{ state.setText("Reverse"); });
   Q::connect(&move, &QPushButton::clicked, &ctl, &Controller::forward);
   Q::connect(&stop, &QPushButton::clicked, &ctl, &Controller::stop);
   ui.show();
   return app.exec();
}

#include "main.moc"

// A rather silly WiringPi mockup
void (*isr)();
int wiringPiSetupSys() { return 0; }
void pinMode(int, int) {}
void digitalWrite(int pin, int value) {
   if (pin == 10 && value == HIGH)
      QTimer::singleShot(1000, isr);
}
int wiringPiISR(int, int, void (*function)()) {
   isr = function;
   return 0;
}
