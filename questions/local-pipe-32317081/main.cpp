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
   static inline int intLen(qint64 len) { return std::min(len, qint64(INT_MAX)); }
   Q_SLOT void _a_write(const QByteArray &data) {
      Q_D(AppPipe);
      if (!(d->openMode & QIODevice::ReadOnly)) return; // We must be readable.
      d->buffer.append(data); // This is a chunk shipped from the source.
      emit hasIncoming(data);
      emit readyRead();
   }
   void hasOutgoingLong(const char *data, qint64 len) {
      while (len) {
         int const size = intLen(len);
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
   bool isSequential() const Q_DECL_OVERRIDE { return true; }
   Q_SIGNAL void hasOutgoing(const QByteArray &);
   Q_SIGNAL void hasIncoming(const QByteArray &);
#if QT_VERSION >= QT_VERSION_CHECK(5,7,0)
   // all the data is in the read buffer already
   qint64 readData(char *, qint64) Q_DECL_OVERRIDE { return 0; }
#else
   qint64 readData(char *data, qint64 len) Q_DECL_OVERRIDE {
      Q_D(AppPipe);
      qint64 hadRead = 0;
      while (len && !d->buffer.isEmpty()) {
         int size = d->buffer.read(data, intLen(len));
         hadRead += size;
         data += size;
         len -= size;
      }
      return hadRead;
   }
   bool canReadLine() const Q_DECL_OVERRIDE {
      Q_D(const AppPipe);
      return d->buffer.indexOf('\n') != -1 || QIODevice::canReadLine();
   }
   qint64 bytesAvailable() const Q_DECL_OVERRIDE {
      Q_D(const AppPipe);
      return QIODevice::bytesAvailable() + d->buffer.size();
   }
   qint64 bytesToWrite() const Q_DECL_OVERRIDE {
      Q_D(const AppPipe);
      return QIODevice::bytesToWrite() + d->writeBuffer.size();
   }
#endif
};

class TestAppPipe : public QObject {
   Q_OBJECT
   QByteArray data1, data2;
   struct PipePair {
      AppPipe end1, end2;
      PipePair(QIODevice::OpenMode mode = QIODevice::NotOpen) :
         end1(QIODevice::ReadWrite | mode), end2(&end1, QIODevice::ReadWrite | mode) {}
   };
   Q_SLOT void initTestCase() {
      data1 = randomData();
      data2 = randomData();
   }
   Q_SLOT void sizes() {
      QCOMPARE(sizeof(AppPipe), sizeof(QIODevice));
   }
   Q_SLOT void basic() {
      PipePair p;
      QVERIFY(p.end1.isOpen() && p.end1.isWritable() && p.end1.isReadable());
      QVERIFY(p.end2.isOpen() && p.end2.isWritable() && p.end2.isReadable());
      static const char hello[] = "Hello There!";
      p.end1.write(hello);
      p.end1.flush();
      QCOMPARE(p.end2.readAll(), QByteArray(hello));
   }
   static QByteArray randomData(int const size = 1024*1024*32) {
      QByteArray data;
      data.resize(size);
      char *const d = data.data();
      for (char *p = d+data.size()-1; p >= d; --p)
         *p = qrand();
      Q_ASSERT(data.size() == size);
      return data;
   }
   static void randomChunkWrite(AppPipe *dev, const QByteArray &payload) {
      for (int written = 0, left = payload.size(); left; ) {
         int const chunk = std::min(qrand() % 82931, left);
         dev->write(payload.mid(written, chunk));
         left -= chunk; written += chunk;
      }
      dev->flush();
   }
   void runBigData(PipePair &p) {
      Q_ASSERT(!data1.isEmpty() && !data2.isEmpty());
      randomChunkWrite(&p.end1, data1);
      randomChunkWrite(&p.end2, data2);
      QCOMPARE(p.end1.bytesAvailable(), qint64(data2.size()));
      QCOMPARE(p.end2.bytesAvailable(), qint64(data1.size()));
      QCOMPARE(p.end1.readAll(), data2);
      QCOMPARE(p.end2.readAll(), data1);
   }
   Q_SLOT void bigDataBuffered() {
      PipePair p;
      runBigData(p);
   }
   Q_SLOT void bigDataUnbuffered() {
      PipePair p(QIODevice::Unbuffered);
      runBigData(p);
   }
   Q_SLOT void cleanupTestCase() {
      data1.clear(); data2.clear();
   }
};

QTEST_MAIN(TestAppPipe)

#include "main.moc"
