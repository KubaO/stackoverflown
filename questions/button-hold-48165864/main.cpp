// https://github.com/KubaO/stackoverflown/tree/master/questions/48165864
#include <QtWidgets>
#include <type_traits>

class Ui : public QWidget {
   Q_OBJECT
   int m_value = -1;
   QStateMachine m_machine{this};
   QState m_idle{&m_machine}, m_active{&m_machine};
   QVBoxLayout m_layout{this};
   QPushButton m_button{"Hold Me"};
   QLabel m_indicator;
   QTimer m_timer;

   void setValue(int val) {
      if (m_value == val) return;
      m_value = val;
      m_indicator.setNum(m_value);
   }
   void step() {
      if (m_value < std::numeric_limits<decltype(m_value)>::max())
         setValue(m_value + 1);
   }
public:
   Ui(QWidget * parent = {}) : QWidget(parent) {
      m_layout.addWidget(&m_button);
      m_layout.addWidget(&m_indicator);
      m_machine.setInitialState(&m_idle);
      m_idle.addTransition(&m_button, &QPushButton::pressed, &m_active);
      m_active.addTransition(&m_button, &QPushButton::released, &m_idle);
      m_machine.start();
      m_timer.setInterval(200);
      connect(&m_timer, &QTimer::timeout, this, &Ui::step);
      connect(&m_active, &QState::entered, [this]{
         step();
         m_timer.start();
      });
      connect(&m_active, &QState::exited, &m_timer, &QTimer::stop);
      setValue(0);
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   Ui ui;
   ui.show();
   return app.exec();
}

#include "main.moc"
