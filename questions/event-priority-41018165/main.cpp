// https://github.com/KubaO/stackoverflown/tree/master/questions/event-priority-41018165
#include <QtCore>
#include <functional>

class Target : public QObject {
    bool event(QEvent *) override;
};

class Event : public QEvent {
    int const priority;
    std::function<void()> fun;
    void process() {
        QThread::msleep(100);
        log.push_back(priority);
        qDebug() << priority;
        if (fun != nullptr) fun();
    }
public:
    static Target target;
    static QVector<int> log;
    Event(int priority, std::function<void()> &&fun = nullptr) :
        QEvent(QEvent::User), priority(priority), fun(std::move(fun)) {}
    static void process(QEvent *event) {
        if (auto e = dynamic_cast<Event*>(event))
            e->process();
    }
    static void post(int priority = Qt::NormalEventPriority) {
        QCoreApplication::postEvent(&target, new Event(priority), priority);
    }
    static void post(std::function<void()> &&fun, int priority = Qt::NormalEventPriority) {
        QCoreApplication::postEvent(&target, new Event(priority, std::move(fun)), priority);
    }
    static void postN(int N, int priority = Qt::NormalEventPriority) {
        for (int i = 0; i < N; ++i) post(priority);
    }
};
Target Event::target;
QVector<int> Event::log;

bool Target::event(QEvent *event)  {
    Event::process(event);
    return true;
}

void post()

int main(int argc, char ** argv) {
    QCoreApplication app{argc, argv};
    Event::postN(3, Qt::LowEventPriority);
    Event::post();
    Event::postN(3, Qt::LowEventPriority-1);
    Event::post(+[]{ qDebug() << "**"; Event::post(); }, Qt::LowEventPriority-1);
    Event::postN(3, Qt::LowEventPriority-1);
    Event::post(+[]{ qDebug() << "*"; QCoreApplication::quit(); }, Qt::LowEventPriority-2);
    app.exec();
    qDebug() << Event::log;
}
