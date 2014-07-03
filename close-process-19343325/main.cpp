#include <QApplication>
#include <QLabel>
#include <QStateMachine>
#include <QBasicTimer>

class Object : public QObject {
    Q_OBJECT
    QBasicTimer m_timer;
    int m_counter;
protected:
    void timerEvent(QTimerEvent * ev) {
        if (ev->timerId() == m_timer.timerId()) {
            m_counter ++;
        }
    }
public:
    Object(QObject * parent = 0) : QObject(parent), m_counter(0) {
        m_timer.start(0, this);
    }
    Q_SLOT void notify() const { emit countedTo(m_counter); }
    Q_SIGNAL void countedTo(int) const;
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    Object object;
    QLabel widget("Close me :)");
    QLabel message("Last window was closed");
    QLabel count;

    QStateMachine machine;
    QState * sWindows = new QState(&machine);
    QState * sSetup = new QState(&machine);
    QState * sMessage = new QState(&machine);
    sWindows->assignProperty(qApp, "quitOnLastWindowClosed", false);
    sWindows->addTransition(qApp, "lastWindowClosed()", sSetup);
    object.connect(sSetup, SIGNAL(entered()), SLOT(notify()));
    count.connect(&object, SIGNAL(countedTo(int)), SLOT(setNum(int)));
    sSetup->addTransition(sMessage);
    sMessage->assignProperty(&message, "visible", true);
    sMessage->assignProperty(&count, "visible", true);
    sMessage->assignProperty(qApp, "quitOnLastWindowClosed", true);
    machine.setInitialState(sWindows);
    machine.start();
    widget.show();
    return app.exec();
}

#include "main.moc"
