#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSet>

class EchoServer : public QTcpServer
{
  QSet<QTcpSocket*> m_pool;
  void incomingConnection(qintptr descr) Q_DECL_OVERRIDE {
    QTcpSocket * s = m_pool.isEmpty() ? new QTcpSocket(this) : *m_pool.begin();
    m_pool.remove(s);
    QObject::connect(s, &QTcpSocket::readyRead, [s]{
      s->write(s->readAll());
    });
    QObject::connect(s, &QTcpSocket::disconnected, [this, s]{
      m_pool.insert(s);
    });
    s->setSocketDescriptor(descr, QTcpSocket::ConnectedState);
  }
public:
  ~EchoServer() { qDebug() << "pool size:" << m_pool.size(); }
};

void setupEchoClient(QTcpSocket & sock)
{
  QObject::connect(&sock, &QTcpSocket::connected, [&sock]{
    int byteCount = 64 + qrand() % 65536;
    sock.setProperty("byteCount", byteCount);
    sock.write(QByteArray(byteCount, '\x2A'));
  });
  QObject::connect(&sock, &QTcpSocket::readyRead, [&sock]{
    int byteCount = sock.property("byteCount").toInt();
    if (byteCount) {
      qint64 read = sock.read(sock.bytesAvailable()).size();
      byteCount -= read;
    }
    if (byteCount <= 0) sock.disconnectFromHost();
    sock.setProperty("byteCount", byteCount);
  });
}

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  QHostAddress addr("127.0.0.1");
  quint16 port = 5050;

  EchoServer server;
  if (! server.listen(addr, port)) qFatal("can't listen");

  QTcpSocket clientSocket;
  setupEchoClient(clientSocket);

  int connectsLeft = 20;
  auto connector = [&clientSocket, &addr, port, &connectsLeft]{
    if (connectsLeft--) {
      qDebug() << "connecting" << connectsLeft;
      clientSocket.connectToHost(addr, port);
    } else
      qApp->quit();
  };
  // reconnect upon disconnection
  QObject::connect(&clientSocket, &QTcpSocket::disconnected, connector);
  // initiate first connection
  connector();

  return a.exec();
}
