// https://github.com/KubaO/stackoverflown/tree/master/questions/miniserial-36431493
#include <QtWidgets>
#include <QtSerialPort>

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   QWidget w;
   QFormLayout layout(&w);
   QPushButton ping("Send");
   QTextBrowser output;
   layout.addRow(&ping);
   layout.addRow(&output);
   w.show();

   QSerialPort port("COM6");
   port.setBaudRate(QSerialPort::Baud9600); // these are guaranteed to return true
   port.setDataBits(QSerialPort::Data8);
   port.setParity(QSerialPort::NoParity);
   port.setStopBits(QSerialPort::OneStop);
   port.setFlowControl(QSerialPort::NoFlowControl);
   if (!port.open(QIODevice::ReadWrite))
      output.append("! Can't open the port :(<br/>");

   QObject::connect(&ping, &QPushButton::clicked, &port, [&]{
      if (port.isOpen()) {
         port.write("1");
         output.append("> 1<br/>");
      } else
         output.append("! Write failed :(<br/>");
   });
   QObject::connect(&port, &QIODevice::readyRead, &output, [&]{
      auto data = port.readAll();
      output.append(QStringLiteral("< %1").arg(QString::fromLatin1(data)));
   });

   return app.exec();
}

