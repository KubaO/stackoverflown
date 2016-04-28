#include <QtCore>
#include <windows.h>

enum { RECEIVE_EVENT_SIZE = 128 };
enum XLstatus { XL_SUCCESS, XL_ERR_QUEUE_IS_EMPTY };
enum XLevent_tag { XL_LIN_MSG };
struct XLevent {
   XLevent_tag tag;
};
typedef int XLhandle;
XLstatus xlReceive(XLhandle, unsigned int *, XLevent *) { return XL_ERR_QUEUE_IS_EMPTY; }

class Device : public QObject {
   Q_OBJECT
   XLhandle m_xlPortHandle;
   QWinEventNotifier m_event;
   HANDLE m_eventHandle;
public:
   Device(QObject * parent = 0) : QObject(parent), m_eventHandle(0) {}
   void open() {
      // open the xl port and get the event handle
      m_event.setHandle(m_eventHandle);
      connect(&m_event, &QWinEventNotifier::activated, this, &Device::handler);
   }
   Q_SIGNAL void dataReceived(const QByteArray &);
private:
   void handler() {
      unsigned int msgsrx = RECEIVE_EVENT_SIZE;
      XLevent xlEvent;
      XLstatus xlStatus;
      while (XL_SUCCESS == (xlStatus = xlReceive(m_xlPortHandle, &msgsrx, &xlEvent))) {
         switch (xlEvent.tag) {
         case XL_LIN_MSG: {
            // ...
            sample();
            emit dataReceived(QByteArray());
            break;
         }
         default:
            qDebug() << "Nothing found";
            break;
         }
      }
   }
   void sample() {}
};

class Thread : public QThread { using QThread::run; public: ~Thread() { quit(); wait(); } };

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);
   Thread deviceThread;
   Device device;
   deviceThread.start();
   device.open();
   device.moveToThread(&deviceThread);
   return a.exec();
}

#include "main.moc"
