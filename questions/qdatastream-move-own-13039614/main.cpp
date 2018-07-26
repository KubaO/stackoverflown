// https://github.com/KubaO/stackoverflown/tree/master/questions/qdatastream-move-own-13039614
#include <QDataStream>

class DataStream : public QDataStream {
   friend class DataStreamTest;
   struct Proxy {
      QScopedPointer<QDataStreamPrivate> d;
      QIODevice *dev;
      bool owndev;
      bool noswap;
      QDataStream::ByteOrder byteorder;
      int ver;
      QDataStream::Status q_status;
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
      virtual ~Proxy();
#endif
   };
   Proxy *p() { return reinterpret_cast<Proxy*>(this); }
   const Proxy *p() const { return reinterpret_cast<const Proxy*>(this); }
public:
   using QDataStream::QDataStream;
   DataStream(DataStream &&other) : DataStream(static_cast<QDataStream&&>(other)) {}
   DataStream(QDataStream &&other) {
      using std::swap;
      Proxy &o = *reinterpret_cast<Proxy*>(&other);
      Proxy &t = *p();
      swap(t.d, o.d);
      swap(t.dev, o.dev);
      swap(t.owndev, o.owndev);
      swap(t.noswap, o.noswap);
      swap(t.byteorder, o.byteorder);
      swap(t.ver, o.ver);
      swap(t.q_status, o.q_status);
   }
   DataStream &operator=(DataStream &&other) { return *this=static_cast<QDataStream&&>(other); }
   DataStream &operator=(QDataStream &&other) {
      this->~DataStream();
      new (this) DataStream(std::move(other));
      return *this;
   }
   void setOwnedDevice(QIODevice *dev) {
      setDevice(dev);
      p()->owndev = true;
   }
   bool ownsDevice() const { return p()->owndev; }
   static bool ownsDevice(const QDataStream *ds) {
      return reinterpret_cast<const Proxy*>(ds)->owndev;
   }
};

#include <QtTest>

class DataStreamTest : public QObject {
   Q_OBJECT
   static QObjectData *getD(QObject *obj) {
      return static_cast<DataStreamTest*>(obj)->d_ptr.data();
   }
   static bool wasDeleted(QObject *obj) {
      return getD(obj)->wasDeleted;
   }
   template <typename T, typename... Args> DataStream make_stream(Args &&...args) {
      return T(std::forward<Args>(args)...);
   }
   Q_SLOT void isBinaryCompatible() {
      QCOMPARE(sizeof(DataStream), sizeof(QDataStream));
      QCOMPARE(sizeof(DataStream::Proxy), sizeof(QDataStream));
   }
   Q_SLOT void streams() {
      QString str{"Hello, world"};
      QVector<uint> ints{44, 0xDEADBEEF, 1};
      QByteArray data;
      DataStream ds(&data, QIODevice::ReadWrite);
      ds << str << ints;
      ds.device()->reset();
      QString str2;
      QVector<uint> ints2;
      ds >> str2 >> ints2;
      QCOMPARE(str2, str);
      QCOMPARE(ints2, ints);
   }
   Q_SLOT void movesFromNotOwnedQDataStream() {
      QBuffer buf;
      QDataStream ds(&buf);
      QVERIFY(ds.device() == &buf);
      DataStream ds2(std::move(ds));
      QVERIFY(!ds.device());
      QVERIFY(ds2.device() == &buf);
      QVERIFY(!wasDeleted(&buf));
   }
   Q_SLOT void movesFromNotOwnedDataStream() {
      QBuffer buf;
      DataStream ds(&buf);
      QVERIFY(ds.device() == &buf);
      DataStream ds2(std::move(ds));
      QVERIFY(!ds.device());
      QVERIFY(ds2.device() == &buf);
      QVERIFY(!wasDeleted(&buf));
   }
   Q_SLOT void assignsFromNotOwnedQDataStream() {
      QBuffer buf;
      QDataStream ds(&buf);
      QVERIFY(ds.device() == &buf);
      DataStream ds2;
      ds2 = std::move(ds);
      QVERIFY(!ds.device());
      QVERIFY(ds2.device() == &buf);
      QVERIFY(!wasDeleted(&buf));
   }
   Q_SLOT void assignsFromNotOwnedDataStream() {
      QBuffer buf;
      DataStream ds(&buf);
      QVERIFY(ds.device() == &buf);
      DataStream ds2;
      ds2 = std::move(ds);
      QVERIFY(!ds.device());
      QVERIFY(ds2.device() == &buf);
      QVERIFY(!wasDeleted(&buf));
   }
   Q_SLOT void returnsFromNotOwnedQDataStream() {
      QBuffer buf;
      {
         auto ds = make_stream<QDataStream>(&buf);
         QVERIFY(ds.device());
         QVERIFY(!ds.ownsDevice());
      }
      QVERIFY(!wasDeleted(&buf));
   }
   Q_SLOT void returnsFromNotOwnedDataStream() {
      QBuffer buf;
      buf.open(QIODevice::ReadWrite);
      {
         auto ds = make_stream<DataStream>(&buf);
         QVERIFY(ds.device());
         QVERIFY(!ds.ownsDevice());
      }
      QVERIFY(!wasDeleted(&buf));
   }
   Q_SLOT void movesFromOwnedQDataStream() {
      QPointer<QIODevice> buf;
      {
         QByteArray data;
         QDataStream ds(&data, QIODevice::ReadWrite);
         QVERIFY(DataStream::ownsDevice(&ds));
         buf = ds.device();
         DataStream ds2(std::move(ds));
         QVERIFY(!ds.device());
         QVERIFY(ds2.device() == buf);
         QVERIFY(buf);
      }
      QVERIFY(!buf);
   }
   Q_SLOT void moveFromOwnedDataStream() {
      QPointer<QBuffer> buf(new QBuffer);
      {
         DataStream ds;
         ds.setOwnedDevice(buf);
         QVERIFY(ds.device() == buf);
         DataStream ds2(std::move(ds));
         QVERIFY(!ds.device());
         QVERIFY(ds2.device() == buf);
         QVERIFY(buf);
      }
      QVERIFY(!buf);
   }
   Q_SLOT void assignsFromOwnedQDataStream() {
      QPointer<QIODevice> buf;
      {
         QByteArray data;
         QDataStream ds(&data, QIODevice::ReadWrite);
         QVERIFY(DataStream::ownsDevice(&ds));
         buf = ds.device();
         DataStream ds2;
         ds2 = std::move(ds);
         QVERIFY(!ds.device());
         QVERIFY(ds2.device() == buf);
         QVERIFY(buf);
      }
      QVERIFY(!buf);
   }
   Q_SLOT void assignsFromOwnedDataStream() {
      QPointer<QBuffer> buf(new QBuffer);
      {
         DataStream ds;
         ds.setOwnedDevice(buf);
         QVERIFY(ds.device() == buf);
         DataStream ds2;
         ds2 = std::move(ds);
         QVERIFY(!ds.device());
         QVERIFY(ds2.device() == buf);
         QVERIFY(buf);
      }
      QVERIFY(!buf);
   }
   Q_SLOT void returnsFromOwnedQDataStream() {
      QPointer<QIODevice> dev;
      QByteArray data;
      {
         auto ds = make_stream<QDataStream>(&data, QIODevice::ReadWrite);
         dev = ds.device();
         QVERIFY(ds.device());
         QVERIFY(ds.ownsDevice());
      }
      QVERIFY(!dev);
   }
   Q_SLOT void returnsFromOwnedDataStream() {
      QPointer<QIODevice> dev;
      QByteArray data;
      {
         auto ds = make_stream<DataStream>(&data, QIODevice::ReadWrite);
         dev = ds.device();
         QVERIFY(ds.device());
         QVERIFY(ds.ownsDevice());
      }
      QVERIFY(!dev);
   }
};

QTEST_MAIN(DataStreamTest)
#include "main.moc"
