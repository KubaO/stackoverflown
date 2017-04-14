// https://github.com/KubaO/stackoverflown/tree/master/questions/socket-split-43409221
#include <QtNetwork>

class SocketBase : public QIODevice {
   Q_OBJECT
public:
   explicit SocketBase(QAbstractSocket * parent) : QIODevice{parent} {
      connect(parent, &QAbstractSocket::connected, this, &SocketBase::connected);
      connect(parent, &QAbstractSocket::disconnected, this, &SocketBase::disconnected);
      connect(parent, &QAbstractSocket::stateChanged, this, [this](QAbstractSocket::SocketState state){
         emit stateChanged(state);
         setOpenMode(m_dev->openMode());
      });
      connect(parent,
              static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
              this, [this](QAbstractSocket::SocketError error){
         setErrorString(m_dev->errorString());
         emit this->error(error);
      });
      setOpenMode(m_dev->openMode());
   }
   QAbstractSocket::SocketError error() const {
      return m_dev->error();
   }
   QAbstractSocket::SocketState state() const {
      return m_dev->state();
   }
   Q_SIGNAL void connected();
   Q_SIGNAL void disconnected();
   Q_SIGNAL void error(QAbstractSocket::SocketError);
   Q_SIGNAL void stateChanged(QAbstractSocket::SocketState);
   bool isSequential() const override { return true; }
protected:
   QAbstractSocket * const m_dev = static_cast<QAbstractSocket*>(parent());
};

class ReadSocket : public SocketBase {
   Q_OBJECT
public:
   explicit ReadSocket(QAbstractSocket * parent) : SocketBase(parent) {
      for (auto proxy : parent->findChildren<ReadSocket*>())
         Q_ASSERT(proxy == this);
      connect(m_dev, &QIODevice::readyRead, this, &QIODevice::readyRead);
   }
   bool atEnd() const override {
      return QIODevice::atEnd() && m_dev->atEnd();
   }
   qint64 bytesAvailable() const override {
      return m_dev->bytesAvailable();
   }
   bool canReadLine() const override {
      return m_dev->canReadLine();
   }
protected:
   qint64 readData(char * data, qint64 maxLength) override {
      return m_dev->read(data, maxLength);
   }
   qint64 readLineData(char *data, qint64 maxLength) override {
      return m_dev->readLine(data, maxLength);
   }
   qint64 writeData(const char *, qint64) override {
      return -1;
   }
};

class WriteSocket : public SocketBase {
   Q_OBJECT
public:
   explicit WriteSocket(QAbstractSocket * parent) : SocketBase(parent) {
      for (auto proxy : parent->findChildren<WriteSocket*>())
         Q_ASSERT(proxy == this);
      connect(m_dev, &QIODevice::bytesWritten, this, &QIODevice::bytesWritten);
   }
   qint64 bytesToWrite() const override {
      return m_dev->bytesToWrite();
   }
   bool flush() {
      return m_dev->flush();
   }
protected:
   qint64 readData(char *, qint64) override {
      return -1;
   }
   qint64 writeData(const char * data, qint64 length) override {
      return m_dev->write(data, length);
   }
};

int main(int argc, char *argv[])
{
   QCoreApplication app{argc, argv};
   QHostAddress addr{"127.0.0.1"};
   quint16 port{9341};

   QTcpServer server;
   if (! server.listen(addr, port)) qFatal("can't listen");
   QObject::connect(&server, &QTcpServer::newConnection, &server, [&]{
      auto s = server.nextPendingConnection();
      QObject::connect(s, &QTcpSocket::readyRead, s, [s]{
         s->write(s->readAll());
      });
      QObject::connect(s, &QTcpSocket::disconnected, s, &QObject::deleteLater);
   });

   const char data_[] = "dhfalksjdfhaklsdhfklasdfs";
   auto const data = QByteArray::fromRawData(data_, sizeof(data_));
   QTcpSocket client;
   WriteSocket writer(&client);
   ReadSocket reader(&client);
   QObject::connect(&writer, &WriteSocket::connected, [&]{
      writer.write(data);
   });
   QObject::connect(&reader, &ReadSocket::readyRead, [&]{
      if (reader.bytesAvailable() >= data.size()) {
         auto const read = reader.read(data.size());
         Q_ASSERT(read == data);
         qApp->quit();
      }
   });
   client.connectToHost(addr, port);
   return app.exec();
}
#include "main.moc"
