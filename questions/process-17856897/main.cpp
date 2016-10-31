// https://github.com/KubaO/stackoverflown/tree/master/questions/process-17856897
#include <QtCore>

QTextStream out{stdout};

class Slave : public QObject {
    QBasicTimer m_timer;
    int m_iter = 0;
    void timerEvent(QTimerEvent * ev) override {
        if (ev->timerId() == m_timer.timerId()) {
            out << "iteration " << m_iter++ << endl;
            if (m_iter > 35) qApp->quit();
        }
    }
public:
    Slave(QObject *parent = nullptr) : QObject(parent) {
        m_timer.start(100, this);
    }
};

class Master : public QObject {
    Q_OBJECT
    QProcess m_proc{this};
    Q_SLOT void read() {
        while (m_proc.canReadLine()) {
            out << "read: " << m_proc.readLine();
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
    Master(QObject *parent = nullptr) : QObject(parent) {
        connect(&m_proc, SIGNAL(readyRead()), SLOT(read()));
        connect(&m_proc, SIGNAL(started()), SLOT(started()));
        connect(&m_proc, SIGNAL(finished(int)), SLOT(finished()));
        m_proc.start(qApp->applicationFilePath(), {"dummy"});
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app{argc, argv};
    if (app.arguments().length() > 1)
        new Slave{&app}; // called with an argument, this is the slave process
    else
        new Master{&app}; // no arguments, this is the master
    return app.exec();
}

#include "main.moc"
