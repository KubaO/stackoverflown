//main.cpp
#include <QApplication>
#include <QThread>
#include <QWidget>
#include <QBasicTimer>
#include <QElapsedTimer>
#include <QGridLayout>
#include <QPlainTextEdit>
#include <QPushButton>

class Helper : private QThread {
public:
    using QThread::usleep;
};

class Trainer : public QObject {
    Q_OBJECT
    Q_PROPERTY(float stopMSE READ stopMSE WRITE setStopMSE)
    float m_stopMSE;
    int m_epochCounter;
    QBasicTimer m_timer;
    void timerEvent(QTimerEvent * ev);
public:
    Trainer(QObject *parent = 0) : QObject(parent), m_stopMSE(1.0) {}
    Q_SLOT void startTraining() {
        m_epochCounter = 0;
        m_timer.start(0, this);
    }
    Q_SLOT void moveToGUIThread() { moveToThread(qApp->thread()); }
    Q_SIGNAL void hasNews(const QString &);
    float stopMSE() const { return m_stopMSE; }
    void setStopMSE(float m) { m_stopMSE = m; }
};

void Trainer::timerEvent(QTimerEvent * ev)
{
    const int updateTime = 50; //ms
    const int maxEpochs = 5000000;
    if (ev->timerId() != m_timer.timerId()) return;

    QElapsedTimer t;
    t.start();
    while (1) {
        // do the work here
        float currentMSE;
#if 0
        for (int i=0; i<zbior_uczacy_rozmiary[0]; i++) {   //internal loop of training
            alg_bp(zbior_uczacy[i], &zbior_uczacy[i][zbior_uczacy_rozmiary[1]]);
            currentMSE += licz_mse(&zbior_uczacy[i][zbior_uczacy_rozmiary[1]]);
        }
#else
        Helper::usleep(100); // pretend we're busy doing some work
        currentMSE = 2E4/m_epochCounter;
#endif
        // bail out if we're done
        if (currentMSE <= m_stopMSE || m_epochCounter >= maxEpochs) {
            QString s = QString::fromUtf8("Zakończono uczenie po %1 epokach, osiągając MSE: %2")
                    .arg(m_epochCounter).arg(currentMSE);
            emit hasNews(s);
            m_timer.stop();
            break;
        }
        // send out periodic updates
        // Note: QElapsedTimer::elapsed() may be expensive, so we don't call it all the time
        if ((m_epochCounter % 128) == 1 && t.elapsed() > updateTime) {
            QString s = QString::fromUtf8("Uczenie w toku, po %1  epokach MSE wynosi: %2")
                        .arg(m_epochCounter).arg(currentMSE);
            emit hasNews(s);
            // return to the event loop if we're in the GUI thread
            if (QThread::currentThread() == qApp->thread()) break; else t.restart();
        }
        m_epochCounter++;
    }
}

class Window : public QWidget {
    Q_OBJECT
    QPlainTextEdit *m_log;
    QThread *m_worker;
    Trainer *m_trainer;
    Q_SIGNAL void startTraining();
    Q_SLOT void showNews(const QString & s) { m_log->appendPlainText(s); }
    Q_SLOT void on_startGUI_clicked() {
        QMetaObject::invokeMethod(m_trainer, "moveToGUIThread");
        emit startTraining();
    }
    Q_SLOT void on_startWorker_clicked() {
        m_trainer->moveToThread(m_worker);
        emit startTraining();
    }
public:
    Window(QWidget *parent = 0, Qt::WindowFlags f = 0) :
        QWidget(parent, f), m_log(new QPlainTextEdit), m_worker(new QThread(this)), m_trainer(new Trainer)
    {
        QGridLayout * l = new QGridLayout(this);
        QPushButton * btn;
        btn = new QPushButton("Start in GUI Thread");
        btn->setObjectName("startGUI");
        l->addWidget(btn, 0, 0, 1, 1);
        btn = new QPushButton("Start in Worker Thread");
        btn->setObjectName("startWorker");
        l->addWidget(btn, 0, 1, 1, 1);
        l->addWidget(m_log, 1, 0, 1, 2);
        connect(m_trainer, SIGNAL(hasNews(QString)), SLOT(showNews(QString)));
        m_trainer->connect(this, SIGNAL(startTraining()), SLOT(startTraining()));
        m_worker->start();
        QMetaObject::connectSlotsByName(this);
    }
    ~Window() {
        m_worker->quit();
        m_worker->wait();
        delete m_trainer;
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Window w;
    w.show();
    return a.exec();
}

#include "main.moc"
