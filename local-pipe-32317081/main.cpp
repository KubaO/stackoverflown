#include <QtWidgets>
#include <private/qringbuffer_p.h>

/// A simple point-to-point intra-process pipe. The other endpoint can live in any
/// thread.
class AppPipe : public QIODevice {
   Q_OBJECT
   AppPipe * m_other { nullptr };
   QRingBuffer m_buf;
   void _a_write(const QByteArray & data) {
      if (! openMode() & QIODevice::ReadOnly) return; // We must be readable.
      m_buf.append(data);
      emit readyRead();
   }
   Q_SIGNAL void _a_writeSig(const QByteArray &);
public:
   AppPipe(AppPipe * other, QIODevice::OpenMode mode, QObject * parent = 0) :
      QIODevice(parent) {
      setOther(other);
      open(mode);
   }
   AppPipe(AppPipe * other, QObject * parent = 0) : QIODevice(parent) {
      setOther(other);
   }
   void setOther(AppPipe * other) {
      if (m_other) disconnect(this, &AppPipe::_a_writeSig, m_other, &AppPipe::_a_write);
      m_other = other;
      if (m_other) connect(this, &AppPipe::_a_writeSig, m_other, &AppPipe::_a_write);
   }
   void close() Q_DECL_OVERRIDE {
      QIODevice::close();
      m_buf.clear();
   }
   qint64 writeData(const char * data, qint64 maxSize) Q_DECL_OVERRIDE {
      if (!maxSize) return maxSize;
      _a_writeSig(QByteArray(data, maxSize));
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
   QCoreApplication a(argc, argv);

   return a.exec();
}

#include "main.moc"
