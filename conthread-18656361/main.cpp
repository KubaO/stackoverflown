#if 1

// Solution with QRunnable
    #include <QCoreApplication>
    #include <QTextStream>
    #include <QRunnable>
    #include <QThreadPool>
    #include <QFile>
    #include <cstdio>

    class Data : public QString {
    public:
        Data(const QString & str) : QString(str) {}
    };

    class Worker : public QRunnable {
        QTextStream m_out;
        Data m_data;
    public:
        void run() {
            // Let's pretend we do something serious with our data here
            m_out << "(" << this << ") " << m_data << endl;
        }
        Worker(const Data & data) : m_out(stdout), m_data(data) {}
    };

    int main(int argc, char *argv[])
    {
        QCoreApplication a(argc, argv);
        QThreadPool * pool = QThreadPool::globalInstance();

        QFile file("file.txt");
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        while (! file.atEnd()) {
            const Data data(file.readLine());
            Worker * worker = new Worker(data);
            pool->start(worker);
        }
        file.close();
        pool->waitForDone();
    }

#else

// Solution with QObject
#include <QCoreApplication>
#include <QTextStream>
#include <QThread>
#include <QFile>
#include <cstdio>

class Notified : public QObject {
    Q_OBJECT
    QTextStream m_out;
public:
    Q_SLOT void notify(const QString & text) {
        m_out << "(" << this << ") " << text << endl;
    }
    Notified(QObject *parent = 0) : QObject(parent), m_out(stdout) {}
};

class Notifier : public QObject {
    Q_OBJECT
    Q_SIGNAL void notification(const QString &);
public:
    Notifier(QObject *parent = 0) : QObject(parent) {}
    void notifyLines(const QString & filePath) {
        QFile file(filePath);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        while (! file.atEnd()) {
            emit notification(file.readLine());
        }
        file.close();
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QObjectList notifieds;
    QList<QThread*> threads;
    Notifier notifier;

    for (int i = 0; i < 4; ++i) {
        QThread * thread = new QThread(&a); // thread owned by the application object
        Notified * notified = new Notified; // can't have an owner before it's moved to another thread
        notified->moveToThread(thread);
        thread->start();
        notifieds << notified;
        threads << thread;
        notified->connect(&notifier, SIGNAL(notification(QString)), SLOT(notify(QString)));
    }

    notifier.notifyLines("file.txt");

    foreach (QThread *thread, threads) {
        thread->quit();
        thread->wait();
    }
    foreach (QObject *notified, notifieds) delete notified;

    a.exit();
}
#include "main.moc"

#endif

#include <QString>
#include <QDataStream>

class C {
    // Everything here is private, the stream operator must be friends!
    bool b;
    QString s;
    C() : b(false) {}
    friend QDataStream & operator << (QDataStream & out, const C & val);
};

QDataStream & operator << (QDataStream & out, const C & val)
{
    out << val.b << val.s;
    return out;
}
