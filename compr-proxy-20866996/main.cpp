#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QFormLayout>

#include <QDebug>

bool metacallShortCircuit(void ** args) {
    // Called as follows:
    // bool result = false;
    // void *cbdata[] = { receiver, event, &result };
    if (reinterpret_cast<QEvent*>(args[1])->type() == QEvent::MetaCall) {
        *reinterpret_cast<bool*>(args[2]) =
                reinterpret_cast<QObject*>(args[0])->event(reinterpret_cast<QEvent*>(args[1]));
        return true;
    }
    return false;
}

class StackUseMonitor {
    Q_DISABLE_COPY(StackUseMonitor)
    char * m_stack;
    int m_recursionLevel;
    friend class StackSample;
    void enter(void * var) {
        if (! m_recursionLevel++) m_stack = (char*)var; else qDebug() << "Stack Use:" << m_stack - (char*)var;
    }
    void leave() { m_recursionLevel--; }
public:
    StackUseMonitor() : m_recursionLevel(0) {}
};

class StackSample {
    Q_DISABLE_COPY(StackSample)
    StackUseMonitor * m_monitor;
public:
    StackSample(StackUseMonitor * monitor) : m_monitor(monitor) { m_monitor->enter(this); }
    ~StackSample() { m_monitor->leave(); }
};

class CompressorProxy : public QObject {
    Q_OBJECT
    StackUseMonitor m_monitor;
    bool emitCheck(bool & flag) {
        StackSample sample(&m_monitor);
        flag = true;
        QCoreApplication::sendPostedEvents(this, QEvent::MetaCall); // recurse
        bool result = flag;
        flag = false;
        return result;
    }

    bool m_slot;
    Q_SLOT void slot() {
        if (emitCheck(m_slot)) emit signal();
    }
    Q_SIGNAL void signal();

    bool m_slot_int;
    Q_SLOT void slot_int(int arg1) {
        if (emitCheck(m_slot_int)) emit signal_int(arg1);
    }
    Q_SIGNAL void signal_int(int);
public:
    CompressorProxy(QObject * parent) : QObject(parent) {}
};

//
// Demo GUI

class Signaller : public QObject {
    Q_OBJECT
public:
    Q_SIGNAL void emptySignal();
    Q_SIGNAL void dataSignal(int);
};

class Widget : public QWidget {
    Q_OBJECT
    QPlainTextEdit * m_edit;
    QSpinBox * m_count;
    Signaller m_signaller;
    Q_SLOT void emptySlot() {
        m_edit->appendPlainText("emptySlot invoked");
    }
    Q_SLOT void dataSlot(int n) {
        m_edit->appendPlainText(QString("dataSlot(%1) invoked").arg(n));
    }
    Q_SLOT void sendSignals() {
        m_edit->appendPlainText(QString("\nEmitting %1 signals").arg(m_count->value()));
        for (int i = 0; i < m_count->value(); ++ i) {
            emit m_signaller.emptySignal();
            emit m_signaller.dataSignal(i + 1);
        }
    }
public:
    Widget(QWidget * parent = 0) : QWidget(parent),
        m_edit(new QPlainTextEdit), m_count(new QSpinBox)
    {
        QFormLayout * l = new QFormLayout(this);
        QPushButton * invoke = new QPushButton("Invoke");
        m_edit->setReadOnly(true);
        m_count->setRange(1, 1000);
        l->addRow("Number of slot invocations", m_count);
        l->addRow(invoke);
        l->addRow(m_edit);
        connect(invoke, SIGNAL(clicked()), SLOT(sendSignals()));
        m_edit->appendPlainText(QString("Qt %1").arg(qVersion()));
        CompressorProxy * proxy = new CompressorProxy(this);
        connect(&m_signaller, SIGNAL(emptySignal()), proxy, SLOT(slot()), Qt::QueuedConnection);
        connect(&m_signaller, SIGNAL(dataSignal(int)), proxy, SLOT(slot_int(int)), Qt::QueuedConnection);
        connect(proxy, SIGNAL(signal()), this, SLOT(emptySlot()));
        connect(proxy, SIGNAL(signal_int(int)), this, SLOT(dataSlot(int)));
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //QInternal::registerCallback(QInternal::EventNotifyCallback, metacallShortCircuit);
    Widget w;
    w.show();
    return a.exec();
}

#include "main.moc"
