// https://github.com/KubaO/stackoverflown/tree/master/questions/serial-controller-40754585
#include <QtWidgets>
#include <QtSerialPort>

class MyController : public QObject {
   Q_OBJECT
public:
   enum IoStatus { Closed, NotReady, Ok, ChecksumError, PortError, Invalid = -1 };
private:
   QSerialPort m_port{this};
   QDataStream m_str{&m_port};
   IoStatus m_ioStatus = Invalid;
   uint8_t m_address, m_status;
   uint32_t m_value;

   bool check(const QByteArray &packet) {
      char checksum = 0;
      for (int i = 0; i < packet.size()-1; ++i)
         checksum += packet [i];
      return checksum == packet[packet.size()-1];
   }
   void onError(QSerialPort::SerialPortError err) {
      if (err != QSerialPort::NoError)
         setIoStatus(PortError);
   }
   void onRxData() {
      if (m_port.bytesAvailable() < 9) return;
      if (m_port.error() != QSerialPort::NoError)
         return;
      if (! check(m_port.peek(9)))
         return setIoStatus(ChecksumError);
      uint8_t dummy;
      m_str >> m_address >> dummy >> m_status >> dummy >> m_value;
      setIoStatus(Ok);
   }
   void setIoStatus(IoStatus ioStatus) {
      if (m_ioStatus == ioStatus) return;
      m_ioStatus = ioStatus;
      emit ioStatusChanged(m_ioStatus);
   }
   static QString text(IoStatus ioStatus) {
      switch (ioStatus) {
      case NotReady: return "Not Ready";
      case Ok: return "Ok";
      case ChecksumError: return "Checksum Error";
      case PortError: return "Serial Port Error";
      default: return "Unknown Status";
      }
   }
public:
   explicit MyController(QObject *parent = nullptr) : QObject(parent) {
      connect(&m_port, &QIODevice::readyRead, this, &MyController::onRxData);
      connect(&m_port, static_cast<void(QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error), this, &MyController::onError);
      m_str.setByteOrder(QDataStream::BigEndian);
      QTimer::singleShot(0, this, [this]{ setIoStatus(Closed); });
   }
   bool open() {
      auto rc = m_port.open(QIODevice::ReadWrite);
      if (rc) setIoStatus(NotReady);
      return rc;
   }
   IoStatus ioStatus() const { return m_ioStatus; }
   QString ioStatusText() const { return text(m_ioStatus); }
   Q_SIGNAL void ioStatusChanged(IoStatus);
   QSerialPort *port() { return &m_port; }
};

int main(int argc, char **argv) {
   QApplication app{argc, argv};
   MyController ctl;

   QWidget w;
   QFormLayout layout{&w};
   QLineEdit port;
   QLabel status;
   QPushButton open{"Open"};
   layout.addRow("Port", &port);
   layout.addRow(&status);
   layout.addRow(&open);

   QObject::connect(&open, &QPushButton::clicked, [&]{
      ctl.port()->setPortName(port.text());
      ctl.open();
   });
   QObject::connect(&ctl, &MyController::ioStatusChanged, [&]{
      status.setText(ctl.ioStatusText());
   });
   w.show();
   return app.exec();
}
#include "main.moc"

#ifndef Q_OS_WIN
using UCHAR = uint8_t;
using HANDLE = void*;
using DWORD = uint32_t;
struct COMSTAT { int cbInQue; };
void ClearCommError(HANDLE, DWORD*, COMSTAT*) {}
void ReadFile(HANDLE, UCHAR*, DWORD, DWORD*, void*) {}
#else
#include <windows.h>
#endif

struct Result {
   uint8_t address;
   uint8_t status;
   uint32_t value;
   enum Status { Ok, NotReady, ChecksumError, ReadError };
   Status ioStatus = Ok;

   QString ioStatusText() const {
      switch (ioStatus) {
      case NotReady: return "Not Ready";
      case Ok: return "Ok";
      case ChecksumError: return "Checksum Error";
      case ReadError: return "Read Error";
      default: return "Unknown Status";
      }
   }
};

Result getResult(HANDLE handle)
{
   Result result;

   DWORD errors, bytesRead;
   COMSTAT comStat;

   ClearCommError(handle, &errors, &comStat);
   if (comStat.cbInQue < 9) {
      result.ioStatus = Result::NotReady;
      return result;
   }

   uint8_t buffer[9];
   ReadFile(handle, buffer, 9, &bytesRead, NULL);

   if (bytesRead < 9) {
      result.ioStatus = Result::ReadError;
      return result;
   }

   uint8_t checksum=0;
   for (int i=0; i<8; i++)
      checksum += buffer[i];

   if (checksum != buffer[8]) {
      result.ioStatus = Result::ChecksumError;
      return result;
   }

   result.address = buffer[0];
   result.status = buffer[2];
   result.value = (buffer[4] << 24) | (buffer[5] << 16) | (buffer[6] << 8) | buffer[7];
   return result;
}

void test() {
    HANDLE handle = 0;
    auto result = getResult(handle);
    printf("%s\n", result.ioStatusText().toLocal8Bit().constData());
}
