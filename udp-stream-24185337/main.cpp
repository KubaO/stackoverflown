#include <QCoreApplication>
#include <QUdpSocket>
#include <QDataStream>
#include <QBasicTimer>

static const quint16 port = 4000;

class MyUDP : public QObject {
   Q_OBJECT
   QUdpSocket m_socket;
   QBasicTimer m_timer;

   void timerEvent(QTimerEvent*ev) {
      if (ev->timerId() != m_timer.timerId()) return;
      sendUDP();
   }
   void sendUDP();
public:
   MyUDP() {
      m_timer.start(1000, this);
      connect(&m_socket, SIGNAL(readyRead()), SLOT(readyRead()));
      m_socket.bind(QHostAddress::LocalHost, port);
   }
   Q_SLOT void readyRead();
};

struct MyStruct {
   int test1;
   bool test2;
   char test3;
   MyStruct() {}
   MyStruct(int t1, bool t2, char t3) : test1(t1), test2(t2), test3(t3) {}
};

template <typename T> T get(QDataStream & str) {
   T value;
   str >> value;
   return value;
}

QDataStream & operator<<(QDataStream & str, const MyStruct & m)
{
   return str << (qint32)m.test1 << (bool)m.test2 << (qint8)m.test3;
}

QDataStream & operator>>(QDataStream & str, MyStruct & m)
{
   m.test1 = get<qint32>(str);
   m.test2 = get<bool>(str);
   m.test3 = get<qint8>(str);
   return str;
}

void MyUDP::sendUDP()
{
   MyStruct envoie(1, true, 97);

   QByteArray buf;
   QDataStream s(&buf, QIODevice::WriteOnly);
   // The encoding is big endian by default, on all systems. You
   // can change it if you wish.
   if (false) s.setByteOrder(QDataStream::LittleEndian);
   s << envoie;

   for (int i = 0; i < 5; ++ i) {
      m_socket.writeDatagram(buf, QHostAddress::LocalHost, port);
   }
}

void MyUDP::readyRead()
{
   QHostAddress sender;
   quint16 senderPort;

   MyStruct recois;
   while (m_socket.hasPendingDatagrams()) {
      QByteArray buf(m_socket.pendingDatagramSize(), Qt::Uninitialized);
      QDataStream str(&buf, QIODevice::ReadOnly);
      m_socket.readDatagram(buf.data(), buf.size(), &sender, &senderPort);
      str >> recois;
      qDebug() << "Message from: " << sender;
      qDebug() << "Message port: " << senderPort;
      qDebug() << "Message: " << recois.test3;
   }
}

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);
   MyUDP udp;
   return a.exec();
}

#include "main.moc"
