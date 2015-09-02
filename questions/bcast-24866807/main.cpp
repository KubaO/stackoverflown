#include <QCoreApplication>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QDebug>

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);
   QUdpSocket socket;

   QList<QHostAddress> ifAddrs = QNetworkInterface::allAddresses();
   qDebug() << ifAddrs;

   QHostAddress ifAddr(QHostAddress::Any);
   foreach (QHostAddress ia, ifAddrs) {
      if (ia.protocol() == QAbstractSocket::IPv6Protocol) continue;
      if (ia.isInSubnet(QHostAddress::LocalHost, 8)) continue;
      ifAddr = ia;
      break;
   }
   if (false) ifAddr = QHostAddress::Any;
   qDebug() << ifAddr;

   if (!socket.bind(ifAddr, 1111)) {
      qDebug() << "bind failed" << socket.error();
   }

   QByteArray d = QString("Hello, world!").toLatin1();
   int r = socket.writeDatagram(d, QHostAddress::Broadcast, 1111);

   qDebug() << r;
   if (r < 0) {
      qDebug() << socket.error();
      qDebug() << socket.errorString();
   }

   return 0;
}
