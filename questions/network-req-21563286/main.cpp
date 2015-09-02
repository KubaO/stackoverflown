#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPointer>
#include <QSslError>
#include <QThread>
#include <QMetaMethod>
#include <QDebug>

class Parser : public QObject {
    Q_OBJECT
    bool m_armed;
    QList<QNetworkReply*> m_replies;
    QPointer<QNetworkAccessManager> m_manager;
    QMetaMethod m_addRequestImpl, m_armImpl;
    Q_SLOT void finished() {
        QNetworkReply * reply = static_cast<QNetworkReply*>(sender());
        Q_ASSERT(m_replies.contains(reply));
        qDebug() << "reply" << reply << "is finished";
        // ... use the data
        m_replies.removeAll(reply);
        if (m_armed && m_replies.isEmpty()) {
            emit finishedAllRequests();
            m_armed = false;
        }
    }
    Q_SLOT void error(QNetworkReply::NetworkError) {
        QNetworkReply * reply = static_cast<QNetworkReply*>(sender());
        m_replies.removeAll(reply);
    }
    Q_SLOT void sslErrors(QList<QSslError>) {
        QNetworkReply * reply = static_cast<QNetworkReply*>(sender());
        m_replies.removeAll(reply);
    }
    Q_INVOKABLE void addRequestImpl(const QNetworkRequest & req) {
        QNetworkReply * reply = m_manager->get(req);
        connect(reply, SIGNAL(finished()), SLOT(finished()));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                SLOT(error(QNetworkReply::NetworkError)));
        connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
                SLOT(sslErrors(QList<QSslError>)));
        m_replies << reply;
    }
    Q_INVOKABLE void armImpl() {
        if (m_replies.isEmpty()) {
            emit finishedAllRequests();
            m_armed = false;
        } else
            m_armed = true;
    }
    static QMetaMethod method(const char * signature) {
        return staticMetaObject.method(staticMetaObject.indexOfMethod(signature));
    }
public:
    // The API is fully thread-safe. The methods can be accessed from any thread.
    explicit Parser(QNetworkAccessManager * nam, QObject * parent = 0) :
        QObject(parent), m_armed(false), m_manager(nam),
        m_addRequestImpl(method("addRequestImpl(QNetworkRequest)")),
        m_armImpl(method("armImpl()"))
    {}
    void addRequest(const QNetworkRequest & req) {
        m_addRequestImpl.invoke(this, Q_ARG(QNetworkRequest, req));
    }
    void arm() {
        m_armImpl.invoke(this);
    }
    Q_SIGNAL void finishedAllRequests();
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QThread * thread = new QThread(&a);
    thread->start();
    QNetworkAccessManager mgr;
    Parser parser(&mgr);
    mgr.moveToThread(thread);
    parser.moveToThread(thread);
    for (int i = 0; i < 10; ++i) {
        QNetworkRequest request(QUrl("https://www.google.pl/"));
        request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");
        parser.addRequest(request);
    }
    thread->connect(&parser, SIGNAL(finishedAllRequests()), SLOT(quit()));
    a.connect(thread, SIGNAL(finished()), SLOT(quit()));
    parser.arm();
    int rc = a.exec();
    thread->wait();
    delete thread; // Otherwise mgr's destruction would fail
    return rc;
}

#include "main.moc"
