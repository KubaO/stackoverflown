// https://github.com/KubaO/stackoverflown/tree/master/questions/thread-jobs-39109247
#include <QtWidgets>
#include <QtConcurrent>
#include <functional>

class Controller : public QObject {
    Q_OBJECT
    QStateMachine m_machine{this};
    QState s_init{&m_machine};
    QState s_busy{&m_machine};
    QState s_idle{&m_machine};
    int m_activeTasks = 0;
    void onTaskStarted() {
        ++ m_activeTasks;
        emit taskRunning();
    }
    void onTaskDone() {
        if (--m_activeTasks == 0) emit allTasksDone();
    }
    Q_SIGNAL void taskRunning();
    Q_SIGNAL void allTasksDone();
    Q_SIGNAL void task1Done(int result);
    Q_SIGNAL void task2Done(int result);
public:
    Q_SIGNAL void active();
    Q_SIGNAL void finished();
    Q_SLOT void doTask1() {
        onTaskStarted();
        QtConcurrent::run([this]{
            QThread::sleep(2); // pretend we do some work
            emit task1Done(42);
        });
    }
    Q_SLOT void doTask2() {
        onTaskStarted();
        QtConcurrent::run([this]{
            QThread::sleep(5); // pretend we do some work
            emit task2Done(44);
        });
    }
    Controller(QObject * parent = nullptr) :
        QObject{parent}
    {
        // This describes the state machine
        s_init.addTransition(this, &Controller::taskRunning, &s_busy);
        s_idle.addTransition(this, &Controller::taskRunning, &s_busy);
        s_busy.addTransition(this, &Controller::allTasksDone, &s_idle);
        m_machine.setInitialState(&s_init);
        m_machine.start();
        //
        connect(this, &Controller::task1Done, this, [this](int result){
            onTaskDone();
            qDebug() << "task 1 is done with result" << result;
        });
        connect(this, &Controller::task2Done, this, [this](int result){
            onTaskDone();
            qDebug() << "task 2 is done with result" << result;
        });
        connect(&s_busy, &QState::entered, this, &Controller::active);
        connect(&s_idle, &QState::entered, this, &Controller::finished);
    }
};

Q_GLOBAL_STATIC(QStringListModel, model)
int main(int argc, char ** argv) {
    using Q = QObject;
    QApplication app{argc, argv};
    Controller ctl;
    QWidget w;
    QFormLayout layout{&w};
    QPushButton start1{"Start Task 1"};
    QPushButton start2{"Start Task 2"};
    QListView log;
    layout.addRow(&start1);
    layout.addRow(&start2);
    layout.addRow(&log);
    Q::connect(&start1, &QPushButton::clicked, &ctl, &Controller::doTask1);
    Q::connect(&start2, &QPushButton::clicked, &ctl, &Controller::doTask2);
    Q::connect(&ctl, &Controller::active, []{ qDebug() << "Active"; });
    Q::connect(&ctl, &Controller::finished, []{ qDebug() << "Finished"; });

    log.setModel(model);
    qInstallMessageHandler(+[](QtMsgType, const QMessageLogContext &, const QString & msg){
        auto row = model->rowCount();
        model->insertRow(row);
        model->setData(model->index(row), msg);
    });
    w.show();
    return app.exec();
}
#include "main.moc"
