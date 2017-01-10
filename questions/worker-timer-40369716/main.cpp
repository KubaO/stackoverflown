// https://github.com/KubaO/stackoverflown/tree/master/questions/worker-timer-40369716
#include <QtCore>
#include <type_traits>

// See http://stackoverflow.com/q/40382820/1329652
template <typename Fun> void safe(QObject * obj, Fun && fun) {
    Q_ASSERT(obj->thread() || qApp && qApp->thread() == QThread::currentThread());
    if (Q_LIKELY(obj->thread() == QThread::currentThread()))
        return fun();
    struct Event : public QEvent {
      using F = typename std::decay<Fun>::type;
      F fun;
      Event(F && fun) : QEvent(QEvent::None), fun(std::move(fun)) {}
      Event(const F & fun) : QEvent(QEvent::None), fun(fun) {}
      ~Event() { fun(); }
    };
    QCoreApplication::postEvent(
          obj->thread() ? obj : qApp, new Event(std::forward<Fun>(fun)));
}

class WorkerBase : public QObject {
    Q_OBJECT
    QBasicTimer timer_;
protected:
    virtual void workUnit() = 0;
    void timerEvent(QTimerEvent *event) override {
        if (event->timerId() == timer_.timerId() && timer_.isActive())
            workUnit();
    }
public:
    using QObject::QObject;
    Q_SIGNAL void finished();
    /// Thread-safe
    Q_SLOT void virtual start() {
        safe(this, [=]{
           timer_.start(0, this);
        });
    }
    /// Thread-safe
    Q_SLOT void virtual stop() {
        safe(this, [=]{
            if (!isActive()) return;
            timer_.stop();
            emit finished();
        });
    }
    bool isActive() const { return timer_.isActive(); }
    ~WorkerBase() {
        if (isActive()) emit finished();
    }
};

class FooWorker : public WorkerBase
{
    Q_OBJECT
    int counter = 0;
    bool isDone() const { return counter >= 10; }
    void workUnit() override {
        if (!isDone()) {
            counter ++;
            emit notify(counter);
            QThread::sleep(1);
        } else
            stop();
    }
public:
    void start() override {
        counter = 0;
        WorkerBase::start();
    }
    void stop() override {
        if (!isDone()) emit aborted();
        WorkerBase::stop();
    }
    Q_SIGNAL void notify(int);
    Q_SIGNAL void aborted();
};

class BarWorker : public WorkerBase
{
    Q_OBJECT
    void workUnit() override {
        QThread::sleep(1);
    }
public:
    void stop() override {
        emit aborted();
        WorkerBase::stop();
    }
    Q_SIGNAL void aborted();
    Q_SLOT void onNotify(int value)
    {
        qDebug() << "Notification value:" << value;
    }
};

class Thread : public QThread { public: ~Thread() { quit(); wait(); } };

int main(int argc, char ** argv) {
    QCoreApplication app{argc, argv};

    BarWorker barWorker;
    FooWorker fooWorker;
    Thread barThread, fooThread;
    barWorker.moveToThread(&barThread);
    fooWorker.moveToThread(&fooThread);
    barWorker.start();
    fooWorker.start();

    QObject::connect(&fooWorker, &FooWorker::finished, &app, &QCoreApplication::quit);
    QObject::connect(&fooWorker, &FooWorker::notify, &barWorker, &BarWorker::onNotify);

    fooThread.start();
    barThread.start();
    return app.exec();
}

#include "main.moc"
