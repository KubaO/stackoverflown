// https://github.com/KubaO/stackoverflown/tree/master/questions/local-pipe-32317081
#include <QtCore>
#include <private/qringbuffer_p.h>

/// A simple point-to-point intra-process pipe. The other endpoint can live in any
/// thread.
class AppPipe : public QIODevice {
   Q_OBJECT
   QRingBuffer m_buf;
   Q_SLOT void _a_write(const QByteArray & data) {
      if (! openMode() & QIODevice::ReadOnly) return; // We must be readable.
      m_buf.append(data);
      emit hasIncoming(data);
      emit readyRead();
   }
public:
   AppPipe(AppPipe * other, QIODevice::OpenMode mode, QObject * parent = {}) :
      AppPipe(other, parent) {
      open(mode);
   }
   AppPipe(AppPipe * other = {}, QObject * parent = {}) : QIODevice(parent) {
      addOther(other);
   }
   void addOther(AppPipe * other) {
      if (other) {
          connect(this, &AppPipe::hasOutgoing, other, &AppPipe::_a_write, Qt::UniqueConnection);
          connect(other, &AppPipe::hasOutgoing, this, &AppPipe::_a_write, Qt::UniqueConnection);
      }
   }
   void removeOther(AppPipe * other) {
      disconnect(this, &AppPipe::hasOutgoing, other, &AppPipe::_a_write);
      disconnect(other, &AppPipe::hasOutgoing, this, &AppPipe::_a_write);
   }
   void close() override {
      QIODevice::close();
      m_buf.clear();
   }
   qint64 writeData(const char * data, qint64 maxSize) override {
      if (maxSize > 0)
         hasOutgoing(QByteArray(data, maxSize));
      return maxSize;
   }
   qint64 readData(char * data, qint64 maxLength) override {
      return m_buf.read(data, maxLength);
   }
   qint64 bytesAvailable() const override {
      return m_buf.size() + QIODevice::bytesAvailable();
   }
   bool canReadLine() const override {
      return QIODevice::canReadLine() || m_buf.canReadLine();
   }
   bool isSequential() const override { return true; }
   Q_SIGNAL void hasOutgoing(const QByteArray &);
   Q_SIGNAL void hasIncoming(const QByteArray &);
};

int main(/*…*/)
{
   /*…*/
   AppPipe end1 { nullptr, QIODevice::ReadWrite };
   AppPipe end2 { &end1, QIODevice::ReadWrite };
   // the pipes are open ready to use
   /*…*/
}

#include "main.moc"
