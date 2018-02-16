// https://github.com/KubaO/stackoverflown/tree/master/questions/sigslot-timing-10838013
//main.cpp
#include <cstdio>
#include <QCoreApplication>
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QTextStream>

static const int n = 1000000;

class Test : public QObject
{
    Q_OBJECT
public slots:
    void trivial(int*, int, int);
    void nonTrivial(QString*, const QString&, const QString&);
signals:
    void trivialSignalD(int*, int, int);
    void nonTrivialSignalD(QString*, const QString&, const QString &);
    void trivialSignalQ(int*, int, int);
    void nonTrivialSignalQ(QString*, const QString&, const QString &);
private slots:
    void run();
private:
    void benchmark(bool timed);
    void testTrivial(void (Test::*)(int*,int,int));
    void testNonTrivial(void (Test::*)(QString*,const QString&, const QString&));
public:
    Test();
};

Test::Test()
{
    connect(this, SIGNAL(trivialSignalD(int*,int,int)),
            SLOT(trivial(int*,int,int)), Qt::DirectConnection);
    connect(this, SIGNAL(nonTrivialSignalD(QString*,QString,QString)),
            SLOT(nonTrivial(QString*,QString,QString)), Qt::DirectConnection);
    connect(this, SIGNAL(trivialSignalQ(int*,int,int)),
            SLOT(trivial(int*,int,int)), Qt::QueuedConnection);
    connect(this, SIGNAL(nonTrivialSignalQ(QString*,QString,QString)),
            SLOT(nonTrivial(QString*,QString,QString)), Qt::QueuedConnection);
    QTimer::singleShot(100, this, SLOT(run()));
}

void Test::run()
{
    // warm up the caches
    benchmark(false);
    // do the benchmark
    benchmark(true);
}

void Test::trivial(int * c, int a, int b)
{
    *c = a + b;
}

void Test::nonTrivial(QString * c, const QString & a, const QString & b)
{
    *c = a + b;
}

void Test::testTrivial(void (Test::* method)(int*,int,int))
{
    static int c;
    int a = 1, b = 2;
    for (int i = 0; i < n; ++i) {
        (this->*method)(&c, a, b);
    }
}

void Test::testNonTrivial(void (Test::* method)(QString*, const QString&, const QString&))
{
    static QString c;
    QString a(500, 'a');
    QString b(500, 'b');
    for (int i = 0; i < n; ++i) {
        (this->*method)(&c, a, b);
    }
}

static int pct(int a, int b)
{
    return (100.0*a/b) - 100.0;
}

void Test::benchmark(bool timed)
{
    const QEventLoop::ProcessEventsFlags evFlags =
            QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers;
    QTextStream out(stdout);
    QElapsedTimer timer;
    quint64 t, nt, td, ntd, ts, nts;

    if (!timed) out << "Warming up the caches..." << endl;

    timer.start();
    testTrivial(&Test::trivial);
    t = timer.elapsed();
    if (timed) out << "trivial direct call took " << t << "ms" << endl;

    timer.start();
    testNonTrivial(&Test::nonTrivial);
    nt = timer.elapsed();
    if (timed) out << "nonTrivial direct call took " << nt << "ms" << endl;

    QCoreApplication::processEvents(evFlags);

    timer.start();
    testTrivial(&Test::trivialSignalD);
    QCoreApplication::processEvents(evFlags);
    td = timer.elapsed();
    if (timed) {
        out << "trivial direct signal-slot call took " << td << "ms, "
               << pct(td, t) << "% longer than direct call." << endl;
    }

    timer.start();
    testNonTrivial(&Test::nonTrivialSignalD);
    QCoreApplication::processEvents(evFlags);
    ntd = timer.elapsed();
    if (timed) {
        out << "nonTrivial direct signal-slot call took " << ntd << "ms, "
               << pct(ntd, nt) << "% longer than direct call." << endl;
    }

    timer.start();
    testTrivial(&Test::trivialSignalQ);
    QCoreApplication::processEvents(evFlags);
    ts = timer.elapsed();
    if (timed) {
        out << "trivial queued signal-slot call took " << ts << "ms, "
               << pct(ts, td) << "% longer than direct signal-slot and "
               << pct(ts, t) << "% longer than direct call." << endl;
    }

    timer.start();
    testNonTrivial(&Test::nonTrivialSignalQ);
    QCoreApplication::processEvents(evFlags);
    nts = timer.elapsed();
    if (timed) {
        out << "nonTrivial queued signal-slot call took " << ts << "ms, "
               << pct(nts, ntd) << "% longer than direct signal-slot and "
               << pct(nts, nt) << "% longer than direct call." << endl;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Test t;
    return a.exec();
}

#include "main.moc"