// https://github.com/KubaO/stackoverflown/tree/master/questions/packet-read-43228728
#include <QtTest>
#include <private/qringbuffer_p.h>

// See http://stackoverflow.com/a/32317276/1329652
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
   AppPipe(AppPipe * other, QIODevice::OpenMode mode, QObject * parent = {}) :
      AppPipe(other, parent) {
      open(mode);
   }
   AppPipe(AppPipe * other = {}, QObject * parent = {}) : QIODevice(parent) {
      addOther(other);
   }
   void addOther(AppPipe * other) {
      if (other) connect(this, &AppPipe::hasOutgoing, other, &AppPipe::_a_write);
   }
   void removeOther(AppPipe * other) {
      disconnect(this, &AppPipe::hasOutgoing, other, &AppPipe::_a_write);
   }
   void close() override {
      QIODevice::close();
      m_buf.clear();
   }
   qint64 writeData(const char * data, qint64 maxSize) override {
      if (!maxSize) return maxSize;
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

class Decoder : public QObject {
   Q_OBJECT
   QPointer<QIODevice> m_device;
   QByteArray m_data;
   char m_first;
   bool m_isFirst = true;
   static constexpr char fromHex(char c) {
      return
            (c >= '0' && c <= '9') ? (c - '0') :
            (c >= 'A' && c <= 'F') ? (c - 'A' + 10) :
            (c >= 'a' && c <= 'f') ? (c - 'a' + 10) :
            -1;
   }
   void decode(const QByteArray & src) {
      for (auto c : src) {
         auto val = fromHex(c);
         if (val < 0) continue;
         if (m_isFirst)
             m_first = val << 4;
         else
             m_data.append(m_first | val);
         m_isFirst = !m_isFirst;
      }
   }
   void onReadyRead() {
      // The data has the format "XX XX XX" where X are hex digits.
      // Spaces and invalid digits are skipped
      decode(m_device->readAll());
      if (m_data.size() >= 4) {
         auto length = 4 + m_data[3];
         if (m_data.size() >= length) {
            emit hasMessage(m_data.left(length));
            m_data.remove(0, length);
         }
      }
   }
public:
   Decoder(QIODevice * dev, QObject * parent = {}) : QObject{parent}, m_device{dev} {
      connect(dev, &QIODevice::readyRead, this, &Decoder::onReadyRead);
   }
   Q_SIGNAL void hasMessage(const QByteArray &);
};

class DecoderTest : public QObject {
   Q_OBJECT
   AppPipe src{nullptr, QIODevice::ReadWrite};
   AppPipe dst{&src, QIODevice::ReadWrite};
   Q_SLOT void initTestCase() {
      src.addOther(&dst);
   }
   Q_SLOT void test1() {
      Decoder dec(&dst, this);
      QSignalSpy spy(&dec, &Decoder::hasMessage);

      src.write("0"); // send a partial header
      QCOMPARE(spy.size(), 0);
      src.write("0 00 00 03 "); // send rest of the header
      QCOMPARE(spy.size(), 0);
      src.write("0A 0B "); // send partial data
      QCOMPARE(spy.size(), 0);
      src.write("0C "); // send rest of data
      QCOMPARE(spy.size(), 1);

      QCOMPARE(dst.bytesAvailable(), 0); // ensure all data has been read

      const QByteArray packet{"\x00\x00\x00\x03\x0A\x0B\x0C", 4+3};
      QCOMPARE(spy.first().size(), 1);
      QCOMPARE(spy.first().first(), {packet});
   }
   Q_SLOT void test2() {
      Decoder dec(&dst, this);
      QSignalSpy spy(&dec, &Decoder::hasMessage);

      src.write("BABE0004 C001 DA7E\n0FAB33"); // send a packet and part of another
      QCOMPARE(spy.size(), 1);
      src.write("01 AB\n");
      QCOMPARE(spy.size(), 2);

      QCOMPARE(spy.at(0).size(), 1);
      QCOMPARE(spy.at(1).size(), 1);
      const QByteArray packet1{"\xBA\xBE\x00\x04\xC0\x01\xDA\x7E", 4+4};
      const QByteArray packet2{"\x0F\xAB\x33\x01\xAB", 4+1};
      QCOMPARE(spy.at(0).first(), {packet1});
      QCOMPARE(spy.at(1).first(), {packet2});
   }
};

QTEST_GUILESS_MAIN(DecoderTest)
#include "main.moc"
