#include <QCoreApplication>
#include <QThreadPool>
#include <QString>
#include <QTimer>
#include <QDebug>

struct ConnectionPtr {};
Q_DECLARE_METATYPE(ConnectionPtr)
struct ConnectionStatus {};
Q_DECLARE_METATYPE(ConnectionStatus)

class CommServer : public QObject, public QRunnable {
    Q_OBJECT
    void run() {
        emit connCallback(ConnectionPtr(), ConnectionStatus());
        emit dataCallback(ConnectionPtr(), "Foo");
    }
public:
    Q_SIGNAL void connCallback(ConnectionPtr, ConnectionStatus);
    Q_SIGNAL void dataCallback(ConnectionPtr, const QString &);
};

class Sprite : public QObject {
    Q_OBJECT
public:
    Sprite(CommServer*) : QObject() {}
    Q_SLOT void connCallbackHandler(ConnectionPtr, ConnectionStatus) {
        qDebug() << __FUNCTION__;
    }
    Q_SLOT void dataCallbackHandler(ConnectionPtr, const QString & str) {
        qDebug() << __FUNCTION__ << str;
    }
    void execute() {}
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    CommServer *cs = new CommServer();
    Sprite *sprite = new Sprite(cs);
    qRegisterMetaType<ConnectionPtr>("ConnectionPtr");
    qRegisterMetaType<ConnectionStatus>("ConnectionStatus");

    QObject::connect(cs, SIGNAL(connCallback(ConnectionPtr,ConnectionStatus)), sprite, SLOT(connCallbackHandler(ConnectionPtr,ConnectionStatus)));
    QObject::connect(cs, SIGNAL(dataCallback(ConnectionPtr,QString)), sprite, SLOT(dataCallbackHandler(ConnectionPtr,QString)));
    sprite->execute();

    QThreadPool::globalInstance()->start(cs);
    QTimer::singleShot(1000, qApp, SLOT(quit()));
    return a.exec();
}

#include "main.moc"
