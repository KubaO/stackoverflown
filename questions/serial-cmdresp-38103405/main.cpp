// https://github.com/KubaO/stackoverflown/tree/master/questions/serial-cmdresp-38103405
#include <QtWidgets>

class Protocol : public QObject {
   Q_OBJECT
   QBasicTimer m_timer;
   QPointer<QIODevice> m_port;
   int m_responseLength = 0;
   int m_read = 0;
   void timerEvent(QTimerEvent * ev) override {
      if (ev->timerId() != m_timer.timerId()) return;
      m_timer.stop();
      emit timedOut();
   }
   void onData() {
      m_read += m_port->bytesAvailable();
      if (m_read < m_responseLength)
         return;
      m_timer.stop();
      emit gotResponse(m_port->read(m_responseLength));
      m_read -= m_responseLength;
      m_responseLength = 0;
   }
public:
   Q_SIGNAL void gotResponse(const QByteArray &);
   Q_SIGNAL void timedOut();
   Q_SLOT void sendCommand(const QByteArray & cmd, int responseLength, int cmdTimeout) {
      m_responseLength = responseLength;
      m_port->write(cmd);
      m_timer.start(cmdTimeout, this);
   }
   explicit Protocol(QIODevice * port, QObject * parent = nullptr) :
      QObject(parent), m_port(port) {
      connect(m_port, &QIODevice::readyRead, this, &Protocol::onData);
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};

   Protocol protocol(0,0);

   Transaction cmd1{"foo", 10, 500, &protocol};
   Transaction cmd2{"bar", 5, 1000, &protocol};
   Transaction cmd3{"baz", 20, 300, &protocol};

   protocol.sendCommand({"foo"}, 10, 500);
   QMetaObject::Connection cmd1;
   cmd1 = QObject::connect(&protocol, &Protocol::gotResponse, [&]{
      QObject::disconnect(cmd1);
      qDebug() << "got response to foo";
   });
   QObject::connect(&protocol, &Protocol::timedOut, []{ qDebug() << "timed out :("; });

   return app.exec();
}

#include "main.moc"
