// https://github.com/KubaO/stackoverflown/tree/master/questions/local-pipe-32317081
// This project is compatible with Qt 4 and Qt 5
#include <QtTest>
#include <private/qiodevice_p.h>
#include <private/qringbuffer_p.h>
#include <algorithm>
#include <climits>

#ifndef Q_DECL_OVERRIDE
#define Q_DECL_OVERRIDE
#endif

class AppPipePrivate : public QIODevicePrivate {
public:
#if QT_VERSION < QT_VERSION_CHECK(5,7,0)
   QRingBuffer buffer;
   QRingBuffer writeBuffer;
   int writeBufferChunkSize;
#endif
   const QByteArray *writeData;
   AppPipePrivate() : writeData(0) { writeBufferChunkSize = 4096; }
};

/// A simple point-to-point intra-process pipe. The other endpoint can live in any
/// thread.
class AppPipe : public QIODevice {
   Q_OBJECT
   Q_DECLARE_PRIVATE(AppPipe)
   Q_SLOT void _a_write(const QByteArray &data) {
      Q_D(AppPipe);
      if (!(d->openMode & QIODevice::ReadOnly)) return; // We must be readable.
      d->buffer.append(data); // This is a chunk shipped from the source.
      emit hasIncoming(data);
      emit readyRead();
   }
   void hasOutgoingLong(const char *data, qint64 len) {
      while (len) {
         qint64 const size = std::min(qint64(INT_MAX), len);
         emit hasOutgoing(QByteArray(data, size));
         data += size;
         len -= size;
      }
   }
public:
   AppPipe(QIODevice::OpenMode mode, QObject *parent = 0) :
      QIODevice(*new AppPipePrivate, parent) {
      open(mode);
   }
   AppPipe(AppPipe *other, QIODevice::OpenMode mode, QObject *parent = 0) :
      QIODevice(*new AppPipePrivate, parent) {
      open(mode);
      addOther(other);
   }
   AppPipe(AppPipe *other, QObject *parent = 0) :
      QIODevice(*new AppPipePrivate, parent) {
      addOther(other);
   }
   AppPipe(QObject *parent = 0) :
      QIODevice(*new AppPipePrivate, parent) {
   }
   ~AppPipe() Q_DECL_OVERRIDE {}
   void addOther(AppPipe *other) {
      if (other) {
         connect(this, SIGNAL(hasOutgoing(QByteArray)), other, SLOT(_a_write(QByteArray)), Qt::UniqueConnection);
         connect(other, SIGNAL(hasOutgoing(QByteArray)), this, SLOT(_a_write(QByteArray)), Qt::UniqueConnection);
      }
   }
   void removeOther(AppPipe *other) {
      disconnect(this, SIGNAL(hasOutgoing(QByteArray)), other, SLOT(_a_write(QByteArray)));
      disconnect(other, SIGNAL(hasOutgoing(QByteArray)), this, SLOT(_a_write(QByteArray)));
   }
   void flush() {
      Q_D(AppPipe);
      while (!d->writeBuffer.isEmpty()) {
         QByteArray const data = d->writeBuffer.read();
         emit hasOutgoing(data);
         emit bytesWritten(data.size());
      }
   }
   void close() Q_DECL_OVERRIDE {
      Q_D(AppPipe);
      flush();
      QIODevice::close();
      d->buffer.clear();
   }
   qint64 write(const QByteArray &data) { // This is an optional optimization. The base method works OK.
      Q_D(AppPipe);
      QScopedValueRollback<const QByteArray*> back(d->writeData);
      if (!(d->openMode & Text))
         d->writeData = &data;
      return QIODevice::write(data);
   }
   qint64 writeData(const char *data, qint64 len) Q_DECL_OVERRIDE {
      Q_D(AppPipe);
      bool buffered = !(d->openMode & Unbuffered);
      if (buffered && (d->writeBuffer.size() + len) > d->writeBufferChunkSize)
         flush();
      if (!buffered
          || len > d->writeBufferChunkSize
          || (len == d->writeBufferChunkSize && d->writeBuffer.isEmpty()))
      {
         if (d->writeData && d->writeData->data() == data && d->writeData->size() == len)
            emit hasOutgoing(*d->writeData);
         else
            hasOutgoingLong(data, len);
      }
      else
         memcpy(d->writeBuffer.reserve(len), data, len);
      return len;
   }
   qint64 readData(char *data, qint64 len) Q_DECL_OVERRIDE {
#if QT_VERSION < QT_VERSION_CHECK(5,7,0)
      Q_D(AppPipe);
      qint64 hadRead = 0;
      while (len && !d->buffer.isEmpty()) {
         qint64 nextSize = d->buffer.nextDataBlockSize();
         int size = d->buffer.read(data, std::min(len, nextSize));
         hadRead += size;
         data += size;
         len -= size;
      }
      return hadRead;
#else
      Q_UNUSED(data); Q_UNUSED(len);
      return 0; // all the data is in the read buffer already
#endif
   }
#if QT_VERSION < QT_VERSION_CHECK(5,7,0)
   bool canReadLine() const Q_DECL_OVERRIDE {
      Q_D(const AppPipe);
      return d->buffer.indexOf('\n') != -1 || QIODevice::canReadLine();
   }
#endif
   bool isSequential() const Q_DECL_OVERRIDE { return true; }
   Q_SIGNAL void hasOutgoing(const QByteArray &);
   Q_SIGNAL void hasIncoming(const QByteArray &);
};

class TestAppPipe : public QObject {
   Q_OBJECT
   Q_SLOT void basic() {
      AppPipe end1(QIODevice::ReadWrite);
      AppPipe end2(&end1, QIODevice::ReadWrite);
      QVERIFY(end1.isOpen() && end1.isWritable() && end1.isReadable());
      QVERIFY(end2.isOpen() && end2.isWritable() && end2.isReadable());
      static const char hello[] = "Hello There!";
      end1.write(hello);
      end1.flush();
      QCOMPARE(end2.readAll(), QByteArray(hello));
   }

};

QTEST_MAIN(TestAppPipe)

#include "main.moc"
