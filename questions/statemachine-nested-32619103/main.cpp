// https://github.com/KubaO/stackoverflown/tree/master/questions/statemachine-nested-32619103
#include <QtCore>

struct OuterState : QState
{
   QStateMachine * machine { nullptr };
   virtual void onEntry(QEvent *) Q_DECL_OVERRIDE
   {
      // through an intervening container
      auto container = new QObject(this);
      machine = new QStateMachine(container);
   }
   OuterState(QState * parent = 0) : QState(parent) {}
};

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);

   // create outer state machine with all states
   QStateMachine outerStateMachine;
   OuterState state1 { &outerStateMachine };
   QFinalState state2 { &outerStateMachine };
   state1.addTransition(&state2);
   outerStateMachine.setInitialState(&state1);
   outerStateMachine.start();

   a.connect(&state1, &QState::entered, []{ qDebug() << "state1 entered"; });
   a.connect(&state2, &QState::entered, []{ qDebug() << "state2 entered"; });
   a.connect(&state2, &QState::entered, qApp, &QCoreApplication::quit);
   return a.exec();
}

