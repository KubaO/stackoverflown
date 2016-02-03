// https://github.com/KubaO/stackoverflown/tree/master/questions/simple-serial-35181906
#include <QtCore>
#include <QtSerialPort>

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   QSerialPort serial;
   serial.setBaudRate(QSerialPort::Baud9600);
   serial.setDataBits(QSerialPort::Data8);
   serial.setParity(QSerialPort::NoParity);
   serial.setStopBits(QSerialPort::OneStop);
   serial.setFlowControl(QSerialPort::NoFlowControl);

   for (auto port : QSerialPortInfo::availablePorts()) {
      serial.setPort(port);
      serial.open(QIODevice::ReadWrite);
      if (serial.isOpen()) {
         qDebug() << "port" << port.portName() << "is open";
         serial.close();
      } else
         qDebug() << "port" << port.portName() << "couldn't be opened";
   }
}
