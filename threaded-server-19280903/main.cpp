#include <QCoreApplication>
#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractEventDispatcher>
#include <QPointer>

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
#define Q_DECL_OVERRIDE override
#endif

// A QThread that quits its event loop upon destruction,
// and waits for the loop to finish.
class StoppingThread : public QThread {
    Q_OBJECT
public:
    StoppingThread(QObject * parent = 0) : QThread(parent) {}
    ~StoppingThread() { quit(); wait(); qDebug() << this; }
};

// Deletes an object living in a thread upon thread's termination.
class ThreadedQObjectDeleter : public QObject {
    Q_OBJECT
    QPointer<QObject> m_object;
    ThreadedQObjectDeleter(QObject * object, QThread * thread) :
        QObject(thread), m_object(object) {}
    ~ThreadedQObjectDeleter() {
        if (m_object && m_object->thread() == 0) {
            delete m_object;
        }
    }
public:
    static void addDeleter(QObject * object, QThread * thread) {
        // The object must not be in the thread yet, otherwise we'd have
        // a race condition.
        Q_ASSERT(thread != object->thread());
        new ThreadedQObjectDeleter(object, thread);
    }
};

// Creates servers whenever the listening server gets a new connection
class ServerFactory : public QObject {
    Q_OBJECT
    QMetaObject m_server;
public:
    ServerFactory(const QMetaObject & client, QObject * parent = 0) :
        QObject(parent), m_server(client) {}
    Q_SLOT void newConnection() {
        QTcpServer * listeningServer = qobject_cast<QTcpServer*>(sender());
        if (!listeningServer) return;
        QTcpSocket * socket = listeningServer->nextPendingConnection();
        if (!socket) return;
        makeServerFor(socket);
    }
protected:
    virtual QObject * makeServerFor(QTcpSocket * socket) {
        QObject * server = m_server.newInstance(Q_ARG(QTcpSocket*, socket), Q_ARG(QObject*, 0));
        socket->setParent(server);
        return server;
    }
};

// A server factory that makes servers in individual threads.
// The threads automatically delete itselves upon finishing.
// Destructing the thread also deletes the server.
class ThreadedServerFactory : public ServerFactory {
    Q_OBJECT
public:
    ThreadedServerFactory(const QMetaObject & client, QObject * parent = 0) :
        ServerFactory(client, parent) {}
protected:
    QObject * makeServerFor(QTcpSocket * socket) Q_DECL_OVERRIDE {
        QObject * server = ServerFactory::makeServerFor(socket);
        QThread * thread = new StoppingThread(this);
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(server, SIGNAL(destroyed()), thread, SLOT(quit()));
        ThreadedQObjectDeleter::addDeleter(server, thread);
        server->moveToThread(thread);
        thread->start();
        return server;
    }
};

// A telnet server with following functionality:
// 1. It echoes everything it receives,
// 2. It shows a smiley face upon receiving CR,
// 3. It quits the server upon ^C
// 4. It disconnects upon receiving 'Q'
class TelnetServer : public QObject {
    Q_OBJECT
    QTcpSocket * m_socket;
    bool m_firstInput;
    Q_SLOT void readyRead() {
        const QByteArray data = m_socket->readAll();
        if (m_firstInput) {
            QTextStream out(m_socket);
            out << "Welcome from thread " << thread() << endl;
            m_firstInput = false;
        }
        for (int i = 0; i < data.length(); ++ i) {
            char c = data[i];
            if (c == '\004') /* ^D */ { m_socket->close(); break; }
            if (c == 'Q') { QCoreApplication::exit(0); break; }
            m_socket->putChar(c);
            if (c == '\r') m_socket->write("\r\n:)", 4);
        }
        m_socket->flush();
    }
public:
    Q_INVOKABLE TelnetServer(QTcpSocket * socket, QObject * parent = 0) :
        QObject(parent), m_socket(socket), m_firstInput(true)
    {
        connect(m_socket, SIGNAL(readyRead()), SLOT(readyRead()));
        connect(m_socket, SIGNAL(disconnected()), SLOT(deleteLater()));
    }
    ~TelnetServer() { qDebug() << this; }
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTcpServer server;
    ThreadedServerFactory factory(TelnetServer::staticMetaObject);
    factory.connect(&server, SIGNAL(newConnection()), SLOT(newConnection()));
    server.listen(QHostAddress::Any, 8023);
    return a.exec();
}

#include "main.moc"
