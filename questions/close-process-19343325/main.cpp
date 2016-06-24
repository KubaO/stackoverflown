// https://github.com/KubaO/stackoverflown/tree/master/questions/close-process-19343325
#include <QtWidgets>

int main(int argc, char** argv)
{
    QApplication app{argc, argv};
    QLabel widget{"Close me :)"};
    QLabel message{"Last window was closed"};

    int counter = 0;
    auto worker = [&]{
       counter++;
    };
    QTimer workerTimer;
    QObject::connect(&workerTimer, &QTimer::timeout, worker);
    workerTimer.start(0);

    QStateMachine machine;
    QState sWindows{&machine};
    QState sSetup  {&machine};
    QState sMessage{&machine};

    sWindows.assignProperty(qApp, "quitOnLastWindowClosed", false);
    sWindows.addTransition(qApp, &QGuiApplication::lastWindowClosed, &sSetup);

    QObject::connect(&sSetup, &QState::entered, [&]{
       workerTimer.stop();
       message.setText(QString("Last window was closed. Count was %1.").arg(counter));
    });
    sSetup.addTransition(&sMessage);

    sMessage.assignProperty(&message, "visible", true);
    sMessage.assignProperty(qApp, "quitOnLastWindowClosed", true);

    machine.setInitialState(&sWindows);
    machine.start();
    widget.show();
    return app.exec();
}

