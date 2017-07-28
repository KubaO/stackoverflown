// https://github.com/KubaO/stackoverflown/tree/master/questions/serial-blocking-45369860
#include <QtSerialPort>

template <class T> bool hasError(const QIODevice & d) {
   return qobject_cast<const T *>(&d) && static_cast<const T &>(d).error() != T::NoError;
}

void chkError(const QIODevice & d) {
   if (hasError<QFile>(d) || hasError<QSerialPort>(d))
      qFatal("I/O Error on %s: %s", d.objectName().toLocal8Bit().constData(),
             d.errorString().toLocal8Bit().constData());
}

void logData(QTextStream & log, const QByteArray & data) {
   log << data.toHex() << "\nLength: " << data.size() << "\n\n";
   log.flush();
   chkError(*log.device());
}

void transmit(QSerialPort & port, const QByteArray & data, QTextStream & log) {
   port.write(data);
   qDebug() << "\nWrote" << data.size() << ":" << data.toHex().constData();
   chkError(port);
   logData(log, data);
}

void receive(QSerialPort & port, QTextStream & log) {
   auto data = port.readAll();
   qDebug() << "\nRead" << data.size() << ":" << data.toHex().constData();
   chkError(port);
   logData(log, data);
}

int main(int argc, char *argv[])
{
   QCoreApplication app(argc, argv);

   QFile logFile("Data.txt");
   if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
      qFatal("Can't open the log file: %s", logFile.errorString().toLocal8Bit().constData());
   QTextStream log(&logFile);

   QSerialPort port;
   port.setPortName("COM2");
   port.setBaudRate(QSerialPort::Baud9600);
   port.setDataBits(QSerialPort::Data8);
   port.setParity(QSerialPort::EvenParity);
   port.setStopBits(QSerialPort::OneStop);
   port.setFlowControl(QSerialPort::NoFlowControl);

   if (!port.open(QIODevice::ReadWrite))
      qFatal("Can't open the serial port: %s", port.errorString().toLocal8Bit().constData());

   logFile.setObjectName("Log File");
   port.setObjectName("Serial Port");

   const QByteArray sendPacket = QByteArrayLiteral("\x01\x03\x00\x00\x00\x0A\0xC5\x0CD");

   while (true) {
      transmit(port, sendPacket, log);
      do {
        receive(port, log);
      } while (port.waitForReadyRead(3000));
   }
}
