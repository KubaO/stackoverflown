#include <QApplication>
#include <QStringList>
#include <QBasicTimer>
#include <QProcess>
#include <QVector>
#include <QDateTime>
#include <QThread>

class Slave : public QObject {
    QBasicTimer m_timer;
    void timerEvent(QTimerEvent * ev) {
        if (ev->timerId() == m_timer.timerId()) qApp->quit();
    }
public:
    Slave(QObject *parent = 0) : QObject(parent) {
        m_timer.start(rand() % 10, this);
    }
};

class Master : public QObject {
    Q_OBJECT
    QList<QProcess> m_processes;
#if 0
    Q_SLOT void started() {
        out << "started" << endl;
    }
    Q_SLOT void finished() {
        out << "finished" << endl;
        qApp->quit();
    }
#endif
public:
    Master(QObject *parent = 0) : QObject(parent) {\
        m_processes.setSharable(false);
        m_processes.clear();
        m_processes[3].start("foo");
#if 0
        connect(m_proc, SIGNAL(started()), SLOT(started()));
        connect(m_proc, SIGNAL(finished(int)), SLOT(finished()));
        m_proc->start(qApp->applicationFilePath(), QStringList("--slave"));
#endif
    }
};

bool argvContains(int argc, char ** argv, const char * needle)
{
    for (int i = 1; i < argc; ++ i) if (qstrcmp(argv[i], needle) == 0) return true;
    return false;
}

int main(int argc, char *argv[])
{
    if (argvContains(argc, argv, "--slave")) {
        QCoreApplication app(argc, argv);
        qsrand(QDateTime::currentMSecsSinceEpoch());
        Slave slave;
        return app.exec();
    }
    QApplication app(argc, argv);
    Master master;
    return app.exec();
}
#include "main.moc"
