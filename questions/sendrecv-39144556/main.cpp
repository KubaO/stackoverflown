// https://github.com/KubaO/stackoverflown/tree/master/questions/sendrecv-39144556
#include <QtWidgets>

enum class Req : quint32 {
    Unknown, String, File
};
Q_DECLARE_METATYPE(Req)
QDataStream & operator<<(QDataStream & ds, Req req) {
    return ds << (quint32)req;
}
QDataStream & operator>>(QDataStream & ds, Req & req) {
    quint32 val;
    ds >> val;
    if (ds.status() == QDataStream::Ok)
        req = Req(val);
    return ds;
}

struct Transaction {
    QDataStream & stream;
    Transaction(QDataStream & stream) : stream{stream} {
        stream.startTransaction();
    }
    ~Transaction() {
        stream.commitTransaction();
    }
    bool ok() {
        return stream.status() == QDataStream::Ok;
    }
};

class Client : public QObject {
    Q_OBJECT
    QIODevice & m_dev;
    QDataStream m_str{&m_dev};
    void onReadyRead() {
        Transaction tr{m_str};
        Req req;
        m_str >> req;
        if (!tr.ok()) return;
        if (req == Req::String)
            emit needString();
        else if (req == Req::File) {
            QString fileName;
            m_str >> fileName;
            if (!tr.ok()) return;
            emit needFile(fileName);
        }
        else emit unknownRequest(req);
    }
public:
    Client(QIODevice & dev) : m_dev{dev} {
        connect(&m_dev, &QIODevice::readyRead, this, &Client::onReadyRead);
    }
    Q_SIGNAL void unknownRequest(Req);
    Q_SIGNAL void needString();
    Q_SIGNAL void needFile(const QString & fileName);
    Q_SLOT void sendString(const QString & str) {
        m_str << Req::String << str;
    }
    Q_SLOT void sendFile(const QString & fileName, const QByteArray & data) {
        m_str << Req::File << fileName << data;
    }
};

class Server : public QObject {
    Q_OBJECT
    QIODevice & m_dev;
    QDataStream m_str{&m_dev};
    void onReadyRead() {
        Transaction tr{m_str};
        Req req;
        m_str >> req;
        if (!tr.ok()) return;
        if (req == Req::String) {
            QString str;
            m_str >> str;
            if (!tr.ok()) return;
            emit hasString(str);
        }
        else if (req == Req::File) {
            QString fileName;
            QByteArray data;
            m_str >> fileName >> data;
            if (!tr.ok()) return;
            emit hasFile(fileName, data);
        }
        else emit hasUnknownRequest(req);
    }
public:
    Server(QIODevice & dev) : m_dev{dev} {
        connect(&m_dev, &QIODevice::readyRead, this, &Server::onReadyRead);
    }
    Q_SIGNAL void hasUnknownRequest(Req);
    Q_SIGNAL void hasString(const QString &);
    Q_SIGNAL void hasFile(const QString & name, const QByteArray &);
    Q_SLOT void requestString() {
        m_str << Req::String;
    }
    Q_SLOT void requestFile(const QString & name) {
        m_str << Req::File << name;
    }
};

int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    return app.exec();
}
#include "main.moc"
