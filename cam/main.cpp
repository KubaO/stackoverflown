#include <cstdio>
#include <limits>
#include <QtCore/QThread>
#include <QtCore/QMutexLocker>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QTimer>
#include <QtCore/QEvent>
#include <QtCore/QElapsedTimer>
#include <QtCore/QTextStream>
#include <QtCore/QCoreApplication>

QTextStream out(stdout);

class TimedBase : public QObject
{
public:
    TimedBase(QObject * parent = 0) : QObject(parent) { reset(); }
    friend QTextStream & operator<<(QTextStream & str, const TimedBase & tb) {
        return str << "avg=" << tb.avg() << "us, max=" << tb.usMax << "us, min="
                   << tb.usMin << "us, n=" << tb.n;
    }
    void reset() { usMax = 0; n = 0; usMin = std::numeric_limits<quint32>::max(); usSum = 0; }
protected:
    quint64 n, usMax, usMin, usSum;
    quint64 avg() const { return (n) ? usSum/n : 0; }
    void tock() {
        const quint64 t = elapsed.nsecsElapsed() / 1000;
        usSum += t;
        if (t > usMax) usMax = t;
        if (t < usMin) usMin = t;
        n ++;
    }
    QElapsedTimer elapsed;
};

class FrameProcessorEvents : public TimedBase
{
    Q_OBJECT
public:
    FrameProcessorEvents(QObject * parent = 0) : TimedBase(parent) {}
public slots: // can be invoked either from object thread or from the caller thread
    void tick() {
        elapsed.start();
        QCoreApplication::postEvent(this, new QEvent(QEvent::User), 1000);
    }
protected:
    void customEvent(QEvent * ev) { if (ev->type() == QEvent::User) tock(); }
};

class FrameProcessorWait : public TimedBase
{
    Q_OBJECT
public:
    FrameProcessorWait(QObject * parent = 0) : TimedBase(parent) {}
    void start() {
        QTimer::singleShot(0, this, SLOT(spinner()));
    }
public: // not a slot since it must be always invoked in the caller thread
    void tick() { elapsed.start(); wc.wakeAll(); }
protected:
    QMutex mutex;
    QWaitCondition wc;
protected slots:
    void spinner() {
        forever {
            QMutexLocker lock(&mutex);
            if (wc.wait(&mutex, 1000)) {
                tock();
            } else {
                return;
            }
        }
    }
};

FrameProcessorEvents * fpe;
FrameProcessorWait * fpw;

static const int avgCount = 1000;
static const int period = 5;

class FrameSender : public QObject
{
    Q_OBJECT
public:
    FrameSender(QObject * parent = 0) : QObject(parent), n(0), N(1) {
        QTimer::singleShot(0, this, SLOT(start()));
    }
protected slots:
    void start() {
        out << (N ? "warming caches..." : "benchmarking...") << endl;
        // fire off a bunch of wait ticks
        n = avgCount;
        timer.disconnect();
        connect(&timer, SIGNAL(timeout()), SLOT(waitTick()));
        fpw->reset();
        fpw->start();
        timer.start(period);
    }
    void waitTick() {
        fpw->tick();
        if (!n--) {
            if (!N) { out << "wait condition latency: " << *fpw << endl; }
            // fire off a bunch of signal+event ticks
            n = avgCount;
            fpe->reset();
            timer.disconnect();
            connect(&timer, SIGNAL(timeout()), fpe, SLOT(tick()));
            connect(&timer, SIGNAL(timeout()), SLOT(signalTick()));
        }
    }
    void signalTick() {
        if (!n--) {
            if (!N) { out << "queued signal connection latency: " << *fpe << endl; }
            // fire off a bunch of event-only ticks
            n = avgCount;
            fpe->reset();
            timer.disconnect();
            connect(&timer, SIGNAL(timeout()), SLOT(eventTick()));
        }
    }
    void eventTick() {
        fpe->tick();
        if (!n--) {
            if (!N) { out << "queued event latency: " << *fpe << endl; }
            if (!N--) {
                qApp->exit();
            } else {
                start();
            }
        }
    }

protected:
    QTimer timer;
    int n, N;
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QThread eThread;
    QThread wThread;
    eThread.start(QThread::TimeCriticalPriority);
    wThread.start(QThread::TimeCriticalPriority);
    fpw = new FrameProcessorWait();
    fpe = new FrameProcessorEvents();
    fpw->moveToThread(&eThread);
    fpe->moveToThread(&wThread);
    FrameSender s;
    a.exec();
    eThread.exit();
    wThread.exit();
    eThread.wait();
    wThread.wait();
    return 0;
}

#include "main.moc"
