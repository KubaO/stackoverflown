#include <QApplication>
#include <QTcpSocket>
#include <QByteArray>

class Sender : public QObject {
    Q_OBJECT
    QIODevice * m_src;
    QAbstractSocket * m_dst;
    QByteArray m_buf;
    qint64 m_hasRead;
    qint64 m_hasWritten;
    qint64 m_srcSize;
    bool m_doneSignaled;
    bool signalDone()  {
        if (!m_doneSignaled &&
                ((m_srcSize && m_hasWritten == m_srcSize) || m_src->atEnd())) {
            emit done();
            m_doneSignaled = true;
        }
        return m_doneSignaled;
    }
    Q_SLOT void dstBytesWritten(qint64 len) {
        if (m_dst->bytesToWrite() < m_buf.size() / 2) {
            // the transmit buffer is running low, refill
            send();
        }
        m_hasWritten += len;
        emit progressed((m_hasWritten * 100) / m_srcSize);
        signalDone();
    }
    Q_SLOT void dstError() {
        emit errorOccurred(tr("Unable to send data. Probably the other side"
                              "cancelled or there are connection problems."));
        qDebug() << m_dst->error();
    }
    void send() {
        if (signalDone()) return;
        qint64 read = m_src->read(m_buf.data(), m_buf.size());
        if (read == -1) {
            emit errorOccurred(tr("Error while reading file."));
            return;
        }
        m_hasRead += read;
        qint64 written = m_dst->write(m_buf.constData(), read);
        if (written == -1) {
            emit errorOccurred(tr("Unable to send data. Probably the other side "
                                  "cancelled or there are connection problems."));
            qDebug() << m_dst->error();
            return;
        }
        if (written != read) {
            emit errorOccurred(tr("Internal error while filling write buffer."));
            qDebug() << m_dst->error();
            return;
        }
    }
public:
    /*! Requires a source device open for reading, and a destination socket open
        for writing. */
    Sender(QIODevice * src, QAbstractSocket * dst, QObject * parent = 0) :
        QObject(parent), m_src(src), m_dst(dst), m_buf(8192, Qt::Uninitialized),
        m_hasRead(0), m_hasWritten(0), m_doneSignaled(false)
    {
        Q_ASSERT(m_src->isReadable());
        Q_ASSERT(m_dst->isWritable());
        connect(m_dst, SIGNAL(bytesWritten(qint64)), SLOT(dstBytesWritten(qint64)));
        connect(m_dst, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(dstError()));
        m_srcSize = m_src->size();
    }
    Q_SLOT void start() { send(); }
    Q_SIGNAL void done();
    Q_SIGNAL void errorOccurred(const QString &);
    Q_SIGNAL void progressed(int percent);
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    return a.exec();
}

#include "main.moc"
