#include <QtCore>
#include <private/qringbuffer_p.h>

/// A simple point-to-point intra-process pipe. The other endpoint can live in any
/// thread.
class AppPipe : public QIODevice {
   Q_OBJECT
   QRingBuffer m_buf;
   void _a_write(const QByteArray & data) {
      if (! openMode() & QIODevice::ReadOnly) return; // We must be readable.
      m_buf.append(data);
      emit hasIncoming(data);
      emit readyRead();
   }
public:
   AppPipe(AppPipe * other, QIODevice::OpenMode mode, QObject * parent = 0) :
      QIODevice(parent) {
      addOther(other);
      open(mode);
   }
   AppPipe(AppPipe * other, QObject * parent = 0) : QIODevice(parent) {
      addOther(other);
   }
   void addOther(AppPipe * other) {
      if (other) connect(this, &AppPipe::hasOutgoing, other, &AppPipe::_a_write);
   }
   void removeOther(AppPipe * other) {
      disconnect(this, &AppPipe::hasOutgoing, other, &AppPipe::_a_write);
   }
   void close() Q_DECL_OVERRIDE {
      QIODevice::close();
      m_buf.clear();
   }
   qint64 writeData(const char * data, qint64 maxSize) Q_DECL_OVERRIDE {
      if (!maxSize) return maxSize;
      hasOutgoing(QByteArray(data, maxSize));
      return maxSize;
   }
   qint64 readData(char * data, qint64 maxLength) Q_DECL_OVERRIDE {
      return m_buf.read(data, maxLength);
   }
   qint64 bytesAvailable() const Q_DECL_OVERRIDE {
      return m_buf.size() + QIODevice::bytesAvailable();
   }
   bool canReadLine() const Q_DECL_OVERRIDE {
      return QIODevice::canReadLine() || m_buf.canReadLine();
   }
   bool isSequential() const Q_DECL_OVERRIDE { return true; }
   Q_SIGNAL void hasOutgoing(const QByteArray &);
   Q_SIGNAL void hasIncoming(const QByteArray &);
};

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);

   return a.exec();
}

#include "main.moc"
