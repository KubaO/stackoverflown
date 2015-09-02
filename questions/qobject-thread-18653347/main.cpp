#include <QApplication>
#include <QVBoxLayout>
#include <QPushButton>
#include <QThread>
#include <QBasicTimer>
#include <QElapsedTimer>
#include <QLabel>
#include <QDebug>

#include <QDataStream>

    struct C {
        bool b;
        QString s;
        C() : b(false) {}
    };

    QDataStream & operator << (QDataStream & out, const C & val)
    {
     out << val.b << val.s;
     return out;
    }

class Helper : private QThread {
public:
    using QThread::usleep;
};

class Worker : public QObject {
    Q_OBJECT
    int m_counter;
    bool m_busy;
    QBasicTimer m_timer;
    void timerEvent(QTimerEvent * ev);
public:
    Worker(QObject *parent = 0) : QObject(parent), m_busy(false) {}
    Q_SLOT void start() {
        if (m_busy) return;
        m_counter = 0;
        m_busy = true;
        m_timer.start(0, this);
    }
    Q_SIGNAL void done();
    Q_SIGNAL void progress(int);
    // must be called from within the working thread, so we wrap it in a slot
    Q_INVOKABLE void moveToThread(QThread *t) { QObject::moveToThread(t); }
};

void Worker::timerEvent(QTimerEvent * ev)
{
    const int busyTime = 50; // [ms] - longest amount of time to stay busy
    const int testFactor = 128; // number of iterations between time tests
    const int maxCounter = 10000;
    if (ev->timerId() != m_timer.timerId()) return;

    QElapsedTimer t;
    t.start();
    while (1) {
        // do some "work"
        Helper::usleep(100);
        m_counter ++;
        // exit when the work is done
        if (m_counter > maxCounter) {
            emit progress(100);
            emit done();
            m_busy = false;
            break;
        }
        // exit when we're done with a timed "chunk" of work
        // Note: QElapsedTimer::elapsed() may be expensive, so we call it once every testFactor iterations
        if ((m_counter % testFactor) == 1 && t.elapsed() > busyTime) {
            emit progress(m_counter*100/maxCounter);
            break;
        }
    }
}

class Window : public QWidget {
    Q_OBJECT
    QLabel *m_label;
    QThread *m_thread;
    QObject *m_worker;
    Q_SIGNAL void start();
    Q_SLOT void showProgress(int p) { m_label->setText(QString("%1 %").arg(p)); }
    void moveWorkerToThread(QThread *thread) {
        qDebug() << QMetaObject::invokeMethod(m_worker, "moveToThread", Q_ARG(QThread*, thread));
    }
    Q_SLOT void on_startGUI_clicked() {
        moveWorkerToThread(qApp->thread());
        emit start();
    }
    Q_SLOT void on_startWorker_clicked() {
        moveWorkerToThread(m_thread);
        emit start();
    }
public:
    Window(QWidget *parent = 0, Qt::WindowFlags f = 0) :
        QWidget(parent, f), m_label(new QLabel), m_thread(new QThread(this)), m_worker(new Worker)
    {
        QVBoxLayout * l = new QVBoxLayout(this);
        QPushButton * btn;
        btn = new QPushButton("Start in GUI Thread");
        btn->setObjectName("startGUI");
        l->addWidget(btn);
        btn = new QPushButton("Start in Worker Thread");
        btn->setObjectName("startWorker");
        l->addWidget(btn);
        l->addWidget(m_label);
        connect(m_worker, SIGNAL(progress(int)), SLOT(showProgress(int)));
        m_worker->connect(this, SIGNAL(start()), SLOT(start()));
        m_thread->start();
        QMetaObject::connectSlotsByName(this);
    }
    ~Window() {
        m_thread->quit();
        m_thread->wait();
        delete m_worker;
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<QThread*>("QThread*"); // for invokeMethod to work
    Window w;
    w.show();
    return a.exec();
}

#include "main.moc"
