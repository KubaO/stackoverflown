// This works on Windows only
#include <QApplication>
#include <QAbstractEventDispatcher>
#include <QWidget>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QFormLayout>
#include <QDebug>

//! Generates application-unique, persistent timer ids.
class TimerIdGenerator : private QAbstractEventDispatcher {
    bool hasPendingEvents() { return false; }
    bool processEvents(QEventLoop::ProcessEventsFlags) { return false; }
    virtual void registerSocketNotifier(QSocketNotifier*) {}
    virtual void unregisterSocketNotifier(QSocketNotifier*) {}
    void registerTimer(int, int, Qt::TimerType, QObject *) {}
    bool unregisterTimer(int) { return false; }
    bool unregisterTimers(QObject *) { return false; }
    QList<TimerInfo> registeredTimers(QObject *) const { return QList<TimerInfo>(); }
    int remainingTime(int) { return 0; }
#ifdef Q_OS_WIN
    bool registerEventNotifier(QWinEventNotifier *) { return false; }
    void unregisterEventNotifier(QWinEventNotifier *) {}
#endif
    void wakeUp() {}
    void interrupt() {}
    void flush() {}
public:
    static int newId();
};
Q_GLOBAL_STATIC(TimerIdGenerator, timerIdGenerator)
int TimerIdGenerator::newId() {
    // Qt uses a nonblocking thread-safe data structure to generate those ids
    return static_cast<QAbstractEventDispatcher*>(timerIdGenerator)->
            registerTimer(0, Qt::CoarseTimer, 0);
}

/*! A timer-identifier-generating wrapper for compressible events.
 * Those events are compressed by Qt and only one exists at a time for
 * a given object's queue.
 * For discussion, see http://stackoverflow.com/a/19189941/1329652 */
class CompressibleBase : public QTimerEvent {
protected:
    CompressibleBase(int timerId) : QTimerEvent(timerId) { qDebug() << timerId; }
};
template <typename Derived> class CompressibleEventWrapper : public CompressibleBase {
public:
    CompressibleEventWrapper() : CompressibleBase(staticTimerId()) {
        Q_STATIC_ASSERT(sizeof(Derived) == sizeof(CompressibleBase)); // We can't carry data
        Q_ASSERT(type() == QEvent::Timer);
    }
    static int staticTimerId() {
        static int id = TimerIdGenerator::newId();
        return id;
    }
    static bool is(const QEvent * ev) {
        return ev->type() == QEvent::Timer &&
                static_cast<const QTimerEvent*>(ev)->timerId() == staticTimerId();
    }
    static Derived* cast(QEvent * ev) { return is(ev) ? static_cast<Derived*>(ev) : 0; }
};

class MyEvent : public CompressibleEventWrapper<MyEvent> {
public:
    MyEvent() { qDebug() << "MyEvent timerId" << timerId(); }
    ~MyEvent() { qDebug() << "deleted"; }
};

class Widget : public QWidget {
    Q_OBJECT
    QFormLayout m_layout;
    QPlainTextEdit m_edit;
    QSpinBox m_count;
    QPushButton m_post;
    bool event(QEvent * event) {
        if (dynamic_cast<MyEvent*>(event)) {
            qDebug() << "rcv";
            m_edit.appendPlainText("MyEvent was received.");
            return true;
        }
        return QWidget::event(event);
    }
    Q_SLOT void sendEvents() {
        m_edit.appendPlainText(QString("\nPosting %1 events").arg(m_count.value()));
        for (int i = 0; i < m_count.value(); ++ i) {
            QCoreApplication::postEvent(this, new MyEvent);
        }
    }
public:
    Widget(QWidget * parent = 0) : QWidget(parent),
        m_layout(this),
        m_post("Post")
    {
        m_edit.setReadOnly(true);
        m_count.setRange(1, 1000);
        m_layout.addRow("Number of events to post", &m_count);
        m_layout.addRow(&m_post);
        m_layout.addRow(&m_edit);
        connect(&m_post, SIGNAL(clicked()), SLOT(sendEvents()));
    }
};

class App : public QApplication {
    bool compressEvent(QEvent * event, QObject *receiver, QPostEventList * list) {
        if (dynamic_cast<MyEvent*>(event)) {
            #ifndef Q_OS_WIN
            // This deadlocks :(
            QCoreApplication::removePostedEvents(receiver, event->type());
            #endif

            bool rc = QApplication::compressEvent(event, receiver, list);
            qDebug() << event << receiver << rc;
            return rc;
        } else {
            return QApplication::compressEvent(event, receiver, list);
        }
    }
public:
    App(int & argc, char ** argv) : QApplication(argc, argv) {}
};

int main(int argc, char *argv[])
{
    App app(argc, argv);
    Widget w;
    w.show();
    return app.exec();
}

#include "main.moc"
