// https://github.com/KubaO/stackoverflown/tree/master/questions/stream-qwidgetlistitem-51403419
#include <QtWidgets>
#include <QtTest>

//

class Layer : public QListWidgetItem {
public:
   virtual QGraphicsItem *it() = 0;
   const QGraphicsItem *it() const { return const_cast<Layer*>(this)->it(); }
   int typeId() const {
      if (type() < UserType)
         return QMetaType::UnknownType;
      return type() - QListWidgetItem::UserType + QMetaType::User;
   }
   const char *typeName() const { return QMetaType::typeName(typeId()); }
   void write(QDataStream&) const override;
   void read(QDataStream&) override;
   QListWidgetItem *clone() const override final;

   void setZValue(int z) { it()->setZValue(z); }
   void setParentItem(Layer *parent) { it()->setParentItem(parent->it()); }
   void setParentItem(QGraphicsItem *parent) { it()->setParentItem(parent); }
   void setSelected(bool sel) { it()->setSelected(sel); }
   void setPos(const QPointF &pos) { it()->setPos(pos); }

   Layer(const Layer &);
   QString name() const { return m_name; }
   void setName(const QString &n) { m_name = n; }
   ~Layer() override = default;
protected:
   using Format = quint8;
   Layer(const QString &name, int typeId);
   static void invalidFormat(QDataStream &);
   template <typename T> T &assign(const T& o) { return static_cast<T&>(assignLayer(o)); }
private:
   QString m_name;
   Layer& assignLayer(const Layer &);
};

//

Layer::Layer(const Layer &o) : Layer(o.name(), o.typeId()) {}

Layer::Layer(const QString &name, int typeId) :
   QListWidgetItem(nullptr, typeId - QMetaType::User + QListWidgetItem::UserType),
   m_name(name)
{}

QListWidgetItem *Layer::clone() const {
   const QMetaType mt(typeId());
   Q_ASSERT(mt.isValid());
   return reinterpret_cast<QListWidgetItem*>(mt.create(this));
}

Layer &Layer::assignLayer(const Layer &o) {
   Q_ASSERT(o.type() == type());
   const QMetaType mt(typeId());
   Q_ASSERT(mt.isValid());
   this->~Layer();
   mt.construct(this, &o);
   return *this;
}

void Layer::write(QDataStream &ds) const {
   ds << typeName() << (Format)0 << m_name << it()->pos();
   QListWidgetItem::write(ds);
}

void Layer::read(QDataStream &ds) {
   QByteArray typeName_;
   Format format_;
   QPointF pos_;
   ds >> typeName_ >> format_;
   if (typeName_.endsWith('\0')) typeName_.chop(1);
   Q_ASSERT(typeName_ == typeName());
   if (format_ >= 0) {
      ds >> m_name >> pos_;
      setPos(pos_);
      QListWidgetItem::read(ds);
   }
   if (format_ >= 1)
      invalidFormat(ds);
}

void Layer::invalidFormat(QDataStream &ds) {
   ds.setStatus(QDataStream::ReadCorruptData);
}

QDataStream &operator<<(QDataStream &ds, const Layer *l) {
   return ds << *l;
}

QByteArray peekByteArray(QDataStream &ds) {
   qint32 size;
   auto read = ds.device()->peek(reinterpret_cast<char*>(&size), sizeof(size));
   if (read != sizeof(size))
      return ds.setStatus(QDataStream::ReadPastEnd), QByteArray();
   if (ds.byteOrder() == QDataStream::BigEndian)
      size = qFromBigEndian(size);
   auto buf = ds.device()->peek(size + 4);
   if (buf.size() != size + 4)
      return ds.setStatus(QDataStream::ReadPastEnd), QByteArray();
   if (buf.endsWith('\0')) buf.chop(1);
   return buf.mid(4);
}

QDataStream &operator>>(QDataStream &ds, Layer *&l) {
   auto typeName = peekByteArray(ds);
   int typeId = QMetaType::type(typeName);
   QMetaType mt(typeId);
   l = mt.isValid() ? reinterpret_cast<Layer*>(mt.create()) : nullptr;
   if (l)
      ds >> *l;
   else
      ds.setStatus(QDataStream::ReadCorruptData);
   return ds;
}

//

class RasterLayer : public Layer, public QGraphicsPixmapItem {
public:
   QGraphicsItem *it() override { return this; }
   int type() const override { return Layer::type(); }
   RasterLayer &operator=(const RasterLayer &o) { return assign(o); }
   void write(QDataStream &) const override;
   void read(QDataStream &) override;
   RasterLayer(const RasterLayer &);
   RasterLayer(const QString &name = {});
};
Q_DECLARE_METATYPE(RasterLayer)

// implementation

static int rasterOps = qRegisterMetaTypeStreamOperators<RasterLayer>();

RasterLayer::RasterLayer(const RasterLayer &o) :
   Layer(o),
   QGraphicsPixmapItem(o.pixmap())
{}

RasterLayer::RasterLayer(const QString &name) : Layer(name, qMetaTypeId<RasterLayer>()) {}

void RasterLayer::write(QDataStream &ds) const {
   Layer::write(ds);
   ds << Format(0) << pixmap();
}

void RasterLayer::read(QDataStream &ds) {
   Layer::read(ds);
   Format format_;
   QPixmap pix_;
   ds >> format_;
   if (format_ >= 0) {
      ds >> pix_;
      setPixmap(pix_);
   }
   if (format_ >= 1)
      invalidFormat(ds);
}

//

class VectorLayer : public Layer, public QGraphicsPathItem {
public:
   QGraphicsItem *it() override { return this; }
   int type() const override { return Layer::type(); }
   VectorLayer &operator=(const VectorLayer &o) { return assign(o); }
   void write(QDataStream &) const override;
   void read(QDataStream &) override;
   VectorLayer(const VectorLayer &);
   VectorLayer(const QString &name = {});
};
Q_DECLARE_METATYPE(VectorLayer)

// implementation

static int vectorOps = qRegisterMetaTypeStreamOperators<VectorLayer>();

VectorLayer::VectorLayer(const VectorLayer &o) :
   Layer(o),
   QGraphicsPathItem(o.path())
{}

VectorLayer::VectorLayer(const QString &name) : Layer(name, qMetaTypeId<VectorLayer>()) {}

void VectorLayer::write(QDataStream &ds) const {
   Layer::write(ds);
   ds << Format(0) << path();
}

void VectorLayer::read(QDataStream &ds) {
   Layer::read(ds);
   Format format_;
   QPainterPath path_;
   ds >> format_;
   if (format_ >= 0) {
      ds >> path_;
      setPath(path_);
   }
   if (format_ >= 1)
      invalidFormat(ds);
}

//

class LayerTest : public QObject {
   Q_OBJECT
   QBuffer buf;
   QDataStream ds{&buf};

private slots:
   void initTestCase() {
      buf.open(QIODevice::ReadWrite);
   }

   void testClone() {
      RasterLayer raster("foo");
      QScopedPointer<QListWidgetItem> clone(raster.clone());
      auto *raster2 = static_cast<RasterLayer*>(clone.data());

      QCOMPARE(raster2->type(), raster.type());
      QCOMPARE(raster2->name(), raster.name());
   }

   void testValueIO() {
      ds.device()->reset();
      RasterLayer raster("foo");
      VectorLayer vector("bar");
      ds << raster << vector;

      ds.device()->reset();
      RasterLayer raster2;
      VectorLayer vector2;
      ds >> raster2 >> vector2;

      QCOMPARE(raster2.name(), raster.name());
      QCOMPARE(vector2.name(), vector.name());
   }

   void testPointerIO() {
      ds.device()->reset();
      RasterLayer raster("foo");
      VectorLayer vector("bar");
      ds << &raster << &vector;

      ds.device()->reset();
      Layer *raster2 = {}, *vector2 = {};
      ds >> raster2 >> vector2;

      QVERIFY(raster2 && vector2);
      QCOMPARE(raster2->typeId(), qMetaTypeId<RasterLayer>());
      QCOMPARE(vector2->typeId(), qMetaTypeId<VectorLayer>());
      QCOMPARE(raster2->name(), raster.name());
      QCOMPARE(vector2->name(), vector.name());
      delete raster2;
      delete vector2;
   }

   void testValueContainerIO() {
      ds.device()->reset();
      QVector<RasterLayer> rasters(2);
      QList<VectorLayer> vectors;
      vectors << VectorLayer() << VectorLayer();
      ds << rasters << vectors;

      ds.device()->reset();
      rasters.clear();
      vectors.clear();
      ds >> rasters >> vectors;

      QCOMPARE(rasters.size(), 2);
      QCOMPARE(vectors.size(), 2);
   }

   void testPointerConteinerIO() {
      ds.device()->reset();
      RasterLayer raster;
      VectorLayer vector;
      QList<Layer*> layers;
      layers << &raster << &vector;
      ds << layers;

      ds.device()->reset();
      layers.clear();
      QVERIFY(layers.isEmpty());
      ds >> layers;
      QCOMPARE(layers.size(), 2);
      QVERIFY(!layers.contains({}));
      qDeleteAll(layers);
   }

   void testVariantIO() {
      ds.device()->reset();
      RasterLayer raster;
      VectorLayer vector;
      auto vr = QVariant::fromValue(raster);
      auto vv = QVariant::fromValue(vector);
      ds << vr << vv;

      ds.device()->reset();
      vv.clear();
      vr.clear();
      QVERIFY(vr.isNull() && vv.isNull());
      ds >> vr >> vv;
      QVERIFY(!vr.isNull() && !vv.isNull());
      QCOMPARE(vr.userType(), qMetaTypeId<RasterLayer>());
      QCOMPARE(vv.userType(), qMetaTypeId<VectorLayer>());
   }

   void testVariantContainerIO() {
      ds.device()->reset();
      QVariantList layers;
      layers << QVariant::fromValue(RasterLayer())
             << QVariant::fromValue(VectorLayer());
      ds << layers;

      ds.device()->reset();
      layers.clear();
      ds >> layers;
      QCOMPARE(layers.size(), 2);
      QVERIFY(!layers.contains({}));
      QCOMPARE(layers.at(0).userType(), qMetaTypeId<RasterLayer>());
      QCOMPARE(layers.at(1).userType(), qMetaTypeId<VectorLayer>());
   }
};

QTEST_MAIN(LayerTest)
#include "main.moc"
