#include <QCoreApplication>
#include <QSerialPort>
#include <QFile>
#include <QTextStream>
#include <QLocalServer>
#include <QLocalSocket>
#include <cstdio>
#include <csignal>

QLocalSocket * xmit;

static void signalHandler(int)
{
    xmit->write(" ");
    xmit->flush();
}

static bool setupSignalHandler()
{
    QLocalServer srv;
    srv.listen("foobarbaz");
    xmit = new QLocalSocket(qApp);
    xmit->connectToServer(srv.serverName(), QIODevice::WriteOnly);
    srv.waitForNewConnection();
    QLocalSocket * receive = srv.nextPendingConnection();
    receive->setParent(qApp);
    qApp->connect(receive, &QLocalSocket::readyRead, &QCoreApplication::quit);
    struct sigaction sig;
    sig.sa_handler = signalHandler;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = SA_RESTART;
    return ! sigaction(SIGINT, &sig, NULL);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    setupSignalHandler();

    QSerialPort port("ttyUSB1");
    QFile file("file.txt");
    QTextStream err(stderr, QIODevice::WriteOnly);
    QTextStream out(stdout, QIODevice::WriteOnly);
    if (!file.open(QIODevice::WriteOnly)) {
        err << "Couldn't open the output file" << endl;
        return 1;
    }
    if (!port.open(QIODevice::ReadWrite)) {
        err << "Couldn't open the port" << endl;
        return 2;
    }
    port.setBaudRate(9600);
    QObject::connect(&port, &QSerialPort::readyRead, [&](){
        QByteArray data = port.readAll();
        out << data;
        file.write(data);
    });
    out << "Use ^C to quit" << endl;
    return a.exec();
}
