#include <QCoreApplication>
#include <QStringList>
#include <QBasicTimer>
#include <QProcess>
#include <QTextStream>

QTextStream out(stdout);

class Slave : public QObject {
    QBasicTimer m_timer;
    int m_i;
    void timerEvent(QTimerEvent * ev) {
        if (ev->timerId() == m_timer.timerId()) {
            out << "iteration " << m_i++ << endl;
            if (m_i > 35) qApp->quit();
        }
    }
public:
    Slave(QObject *parent = 0) : QObject(parent), m_i(0) {
        m_timer.start(100, this);
    }
};

class Master : public QObject {
    Q_OBJECT
    QProcess * m_proc;
    Q_SLOT void read() {
        while (m_proc->canReadLine()) {
            out << "read: " << m_proc->readLine();
            out.flush(); // endl implicitly flushes, so we must do the same
        }
    }
    Q_SLOT void started() {
        out << "started" << endl;
    }
    Q_SLOT void finished() {
        out << "finished" << endl;
        qApp->quit();
    }
public:
    Master(QObject *parent = 0) : QObject(parent), m_proc(new QProcess(this)) {
        connect(m_proc, SIGNAL(readyRead()), SLOT(read()));
        connect(m_proc, SIGNAL(started()), SLOT(started()));
        connect(m_proc, SIGNAL(finished(int)), SLOT(finished()));
        m_proc->start(qApp->applicationFilePath(), QStringList("dummy"));
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    if (a.arguments().length() > 1) {
        new Slave(&a); // called with an argument, this is the slave process
    } else {
        new Master(&a); // no arguments, this is the master
    }
    return a.exec();
}

#include "main.moc"
