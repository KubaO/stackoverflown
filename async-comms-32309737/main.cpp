#include <QtWidgets>
#include <private/qringbuffer_p.h>

/// A simple point-to-point intra-application pipe. This class is not thread-safe.
class AppPipe : public QIODevice {
   Q_OBJECT
   AppPipe * m_other { nullptr };
   QRingBuffer m_buf;
public:
   AppPipe(AppPipe * other, QObject * parent = 0) : QIODevice(parent), m_other(other) {
      open(QIODevice::ReadWrite);
   }
   void setOther(AppPipe * other) { m_other = other; }
   qint64 writeData(const char * data, qint64 maxSize) Q_DECL_OVERRIDE {
      if (!maxSize) return maxSize;
      m_other->m_buf.append(QByteArray(data, maxSize));
      emit m_other->readyRead();
      return maxSize;
   }
   qint64 readData(char * data, qint64 maxLength) Q_DECL_OVERRIDE {
      return m_buf.read(data, maxLength);
   }
   qint64 bytesAvailable() const Q_DECL_OVERRIDE {
      return m_buf.size() + QIODevice::bytesAvailable();
   }
   bool isSequential() const Q_DECL_OVERRIDE { return true; }
};

int main(int argc, char *argv[])
{
   QApplication a { argc, argv };
   QWidget ui;
   QGridLayout grid { &ui };
   QLabel state;
   QPushButton restart { "Restart" }, transmit { "Transmit" };
   grid.addWidget(&state, 0, 0, 1, 2);
   grid.addWidget(&restart, 1, 0);
   grid.addWidget(&transmit, 1, 1);
   ui.show();

   AppPipe device { nullptr };
   AppPipe client { &device };
   device.setOther(&client);
   QStateMachine sm;
   QState
         s_init { &sm },    // Exited after a delay
         s_wait { &sm },    // Waits for data to arrive
         s_end { &sm };     // Final state
   QTimer timer;
   timer.setSingleShot(true);

   sm.setInitialState(&s_init);
   QObject::connect(&sm, &QStateMachine::stopped, &sm, &QStateMachine::start);
   QObject::connect(&s_init, &QState::entered, [&]{ timer.start(1500); });
   s_init.addTransition(&timer, SIGNAL(timeout()), &s_wait);
   s_wait.addTransition(&client, SIGNAL(readyRead()), &s_end);

   s_init.assignProperty(&state, "text", "Waiting for timeout.");
   s_wait.assignProperty(&state, "text", "Waiting for data.");
   s_end.assignProperty(&state, "text", "Done.");

   QObject::connect(&restart, &QPushButton::clicked, &sm, &QStateMachine::stop);
   QObject::connect(&transmit, &QPushButton::clicked, [&]{
      device.write("*", 1);
   });

   sm.start();
   return a.exec();
}

#include "main.moc"
