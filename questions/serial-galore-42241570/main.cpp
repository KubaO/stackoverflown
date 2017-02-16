// https://github.com/KubaO/stackoverflown/tree/master/questions/serial-galore-42241570
#include <QtWidgets>
#include <QtSerialPort>

// See http://stackoverflow.com/q/40382820/1329652
template <typename Fun> void safe(QObject * obj, Fun && fun) {
   Q_ASSERT(obj->thread() || qApp && qApp->thread() == QThread::currentThread());
   if (Q_LIKELY(obj->thread() == QThread::currentThread() || !obj->thread()))
      return fun();
   struct Event : public QEvent {
      using F = typename std::decay<Fun>::type;
      F fun;
      Event(F && fun) : QEvent(QEvent::None), fun(std::move(fun)) {}
      Event(const F & fun) : QEvent(QEvent::None), fun(fun) {}
      ~Event() { fun(); }
   };
   QCoreApplication::postEvent(
            obj->thread() ? obj : qApp, new Event(std::forward<Fun>(fun)));
}

class SerialController : public QObject {
   Q_OBJECT
   QSerialPort m_port{this};
   QByteArray m_rxData;

   void onError(QSerialPort::SerialPortError error) {
      Q_UNUSED(error);
   }
   void onData(const QByteArray & data) {
      m_rxData.append(data);
      qDebug() << "Got" << m_rxData.toHex() << "(" << m_rxData.size() << ") - done.";
      emit hasReply(m_rxData);
   }
   void onData() {
      if (m_port.bytesAvailable() >= 4)
         onData(m_port.readAll());
   }
public:
   explicit SerialController(const QString & port, QObject * parent = nullptr) :
      QObject{parent}
   {
      m_port.setPortName(port);
      connect(&m_port, static_cast<void(QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
              this, &SerialController::onError);
   }
   ~SerialController() { qDebug() << __FUNCTION__; }
   bool open() {
      m_port.setBaudRate(QSerialPort::Baud9600);
      m_port.setDataBits(QSerialPort::Data8);
      m_port.setParity(QSerialPort::NoParity);
      m_port.setStopBits(QSerialPort::OneStop);
      m_port.setFlowControl(QSerialPort::NoFlowControl);
      return m_port.open(QIODevice::ReadWrite);
   }
   /// This method is thread-safe.
   void start() {
      safe(this, [=]{
         m_port.write("S");
         qDebug() << "Sent data";
      });
   }
   Q_SIGNAL void hasReply(const QByteArray &);
   void injectData(const QByteArray & data) {
      onData(data);
   }
};

QDebug operator<<(QDebug dbg, const QSerialPortInfo & info) {
   dbg << info.portName();
   if (!info.description().isEmpty())
       dbg << " Description: " <<  info.description();
   if (!info.manufacturer().isEmpty())
       dbg << " Manufacturer: " <<  info.manufacturer();
   return dbg;
}

// A thread that starts on construction, and is always safe to destruct.
class RunningThread : public QThread {
   Q_OBJECT
   using QThread::run; // final
public:
   RunningThread(QObject * parent = nullptr) : QThread(parent) { start(); }
   ~RunningThread() { qDebug() << __FUNCTION__; quit(); wait(); }
};

int main(int argc, char *argv[])
{
   QCoreApplication app(argc, argv);
   auto const ports = QSerialPortInfo::availablePorts();
   if (ports.isEmpty())
      qFatal("No serial ports");

   int n{};
   qDebug() << "Available ports:";
   for (auto & port : ports)
      qDebug() << "port[" << n++ << "]: " << port;

   SerialController ctl{ports.at(5).portName()};
   if (!ctl.open())
      qFatal("Open Failed");

   // Optional: the controller will work fine in the main thread.
   if (true) ctl.moveToThread(new RunningThread{&ctl}); // Owns its thread

   // Let's pretend we got a reply;
   QTimer::singleShot(1000, &ctl, [&ctl]{
      ctl.injectData("ABCD");
   });
   QObject::connect(&ctl, &SerialController::hasReply, ctl.thread(), &QThread::quit);
   QObject::connect(&ctl, &SerialController::hasReply, [&]{
      qDebug() << "The controller is done, quitting.";
      app.quit();
   });
   ctl.start();
   return app.exec();
}
#include "main.moc"
